/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__UTILITY__SIGNAL_H__INCLUDED
#define HARUHI__UTILITY__SIGNAL_H__INCLUDED

// Standard:
#include <cstddef>
#include <functional>
#include <algorithm>
#include <list>

// Lib:
#include <boost/function.hpp>

// Haruhi:
#include <haruhi/config/all.h>


namespace Signal {

namespace Private {

	class DefaultConnectionBase;


	class SignalBase
	{
	  public:
		virtual void disconnect (DefaultConnectionBase*) = 0;
	};


	class DefaultConnectionBase
	{
	  public:
		DefaultConnectionBase (SignalBase* signal) noexcept:
			_signal (signal)
		{ }

		virtual ~DefaultConnectionBase() noexcept = default;

		SignalBase*
		signal() const noexcept
		{
			return _signal;
		}

	  private:
		SignalBase* _signal;
	};


	typedef std::list<DefaultConnectionBase*> Connections;

} // namespace Private


template<class ...Argument>
	class Emiter;


class Receiver
{
	template<class ...Argument>
		friend class Emiter;

  public:
	virtual ~Receiver()
	{
		disconnect_all_signals();
	}

  protected:
	/**
	 * Disconnects all signals pointing to this receiver. Useful in multithreaded
	 * programs when one would want to ensure that Receiver is "disconnected"
	 * at the beginning of its destructor.
	 */
	virtual void
	disconnect_all_signals() noexcept
	{
		Private::Connections::iterator i = _connections.begin();
		// Receiver doesn't manage its _connections. Signals do.
		while (i != _connections.end())
		{
			(*i)->signal()->disconnect (*i);
			i = _connections.begin();
		}
	}

  private:
	Private::Connections _connections;
};


template<class ...Argument>
	class Emiter: public Private::SignalBase
	{
		class ConnectionBase: public Private::DefaultConnectionBase
		{
		  public:
			ConnectionBase (SignalBase* signal, Receiver* receiver) noexcept:
				Private::DefaultConnectionBase (signal),
				_base_r (receiver)
			{ }

			virtual void call (Argument	...arg) = 0;

			Receiver*
			receiver() const noexcept
			{
				return _base_r;
			}

		  private:
			Receiver* _base_r;
		};

		template<class Receiver>
			class Connection: public ConnectionBase
			{
				typedef void (Receiver::*Method)(Argument ...arg);

			  public:
				Connection (SignalBase* signal, Receiver* r, Method m) noexcept:
					ConnectionBase (signal, r),
					_r (r),
					_m (m)
				{ }

				void
				call (Argument ...arg) override
				{
					(_r->*_m)(arg...);
				}

				bool
				is (Receiver* r, Method m) noexcept
				{
					return r == _r && m == _m;
				}

			  private:
				Receiver* _r;
				Method _m;
			};

		template<class Receiver>
			class BoostFunctionConnection: public ConnectionBase
			{
				typedef boost::function<void (Argument...)> Callback;

			  public:
				BoostFunctionConnection (SignalBase* signal, Callback c) noexcept:
					_c (c)
				{ }

				void
				call (Argument ...arg) override
				{
					_c (arg...);
				}

			  private:
				Callback _c;
			};

		typedef std::list<ConnectionBase*> Connections;

	  public:
		virtual ~Emiter() noexcept
		{
			for (ConnectionBase* c: _connections)
			{
				c->receiver()->_connections.remove (c);
				delete c;
			}

			_connections.clear();
		}

		template<class Receiver>
			void
			connect (Receiver* receiver, void (Receiver::*method)(Argument...))
			{
				auto connection = new Connection<Receiver> (this, receiver, method);
				_connections.push_back (connection);
				receiver->_connections.push_back (connection);
			}

		template<class Receiver>
			void
			connect (boost::function<void (Argument...)> const& function)
			{
				_connections.push_back (new BoostFunctionConnection<Receiver> (this, function));
			}

		template<class Receiver>
			void
			disconnect (Receiver* receiver, void (Receiver::*method)(Argument...)) noexcept
			{
				for (ConnectionBase* c: _connections)
				{
					if (auto connection = dynamic_cast<Connection<Receiver>*> (c))
					{
						if (connection->is (receiver, method))
						{
							disconnect (connection);
							break;
						}
					}
				}
			}

		void
		operator() (Argument ...arg)
		{
			for (auto& c: _connections)
				c->call (arg...);
		}

		auto
		connections_number() const noexcept
		{
			return _connections.size();
		}

	  protected:
		void
		disconnect (Private::DefaultConnectionBase* connection) noexcept override
		{
			typename Connections::iterator i = std::find (_connections.begin(), _connections.end(), connection);

			if (i != _connections.end())
			{
				(*i)->receiver()->_connections.remove (connection);
				delete *i;

				_connections.erase (i);
			}
		}

	  private:
		Connections _connections;
	};

} // namespace Signal

#endif

