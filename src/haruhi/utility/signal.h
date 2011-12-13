/* vim:ts=4
 *
 * Copyleft 2008…2011  Michał Gawron
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


namespace Signal {

namespace Private {

	class ConnectionBase;


	class SignalBase
	{
	  public:
		virtual void disconnect (ConnectionBase*) = 0;
	};


	class ConnectionBase
	{
	  public:
		ConnectionBase (SignalBase* signal):
			_signal (signal)
		{ }

		virtual ~ConnectionBase()
		{ }

		SignalBase*
		signal() const
		{
			return _signal;
		}

	  private:
		SignalBase* _signal;
	};


	typedef std::list<ConnectionBase*> Connections;

} // namespace Private


class Emiter0;
template<class> class Emiter1;
template<class, class> class Emiter2;
template<class, class, class> class Emiter3;
template<class, class, class, class> class Emiter4;
template<class, class, class, class, class> class Emiter5;
template<class, class, class, class, class, class> class Emiter6;
template<class, class, class, class, class, class, class> class Emiter7;
template<class, class, class, class, class, class, class, class> class Emiter8;
template<class, class, class, class, class, class, class, class, class> class Emiter9;

class Receiver
{
	friend class Emiter0;
	template<class> friend class Emiter1;
	template<class, class> friend class Emiter2;
	template<class, class, class> friend class Emiter3;
	template<class, class, class, class> friend class Emiter4;
	template<class, class, class, class, class> friend class Emiter5;
	template<class, class, class, class, class, class> friend class Emiter6;
	template<class, class, class, class, class, class, class> friend class Emiter7;
	template<class, class, class, class, class, class, class, class> friend class Emiter8;
	template<class, class, class, class, class, class, class, class, class> friend class Emiter9;

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
	disconnect_all_signals()
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

} // namespace Signal


/*
 * Instantiate signals
 */


#define EMITER_INSTANTIATE


#define EMITER_CLASS Emiter0
#define EMITER_PARAMETERS_LIST
#define EMITER_PARAMETER_TYPES_LIST
#define EMITER_ARGUMENTS_LIST

#include "signal.h"

#undef EMITER_CLASS
#undef EMITER_CLASS_TEMPLATE_PARAMETERS
#undef EMITER_PARAMETERS_LIST
#undef EMITER_PARAMETER_TYPES_LIST
#undef EMITER_ARGUMENTS_LIST


#define EMITER_CLASS Emiter1
#define EMITER_CLASS_TEMPLATE_PARAMETERS class A1
#define EMITER_PARAMETERS_LIST A1 a1
#define EMITER_PARAMETER_TYPES_LIST A1
#define EMITER_ARGUMENTS_LIST a1

#include "signal.h"

#undef EMITER_CLASS
#undef EMITER_CLASS_TEMPLATE_PARAMETERS
#undef EMITER_PARAMETERS_LIST
#undef EMITER_PARAMETER_TYPES_LIST
#undef EMITER_ARGUMENTS_LIST


#define EMITER_CLASS Emiter2
#define EMITER_CLASS_TEMPLATE_PARAMETERS class A1, class A2
#define EMITER_PARAMETERS_LIST A1 a1, A2 a2
#define EMITER_PARAMETER_TYPES_LIST A1, A2
#define EMITER_ARGUMENTS_LIST a1, a2

#include "signal.h"

#undef EMITER_CLASS
#undef EMITER_CLASS_TEMPLATE_PARAMETERS
#undef EMITER_PARAMETERS_LIST
#undef EMITER_PARAMETER_TYPES_LIST
#undef EMITER_ARGUMENTS_LIST


#define EMITER_CLASS Emiter3
#define EMITER_CLASS_TEMPLATE_PARAMETERS class A1, class A2, class A3
#define EMITER_PARAMETERS_LIST A1 a1, A2 a2, A3 a3
#define EMITER_PARAMETER_TYPES_LIST A1, A2, A3
#define EMITER_ARGUMENTS_LIST a1, a2, a3

#include "signal.h"

#undef EMITER_CLASS
#undef EMITER_CLASS_TEMPLATE_PARAMETERS
#undef EMITER_PARAMETERS_LIST
#undef EMITER_PARAMETER_TYPES_LIST
#undef EMITER_ARGUMENTS_LIST


#define EMITER_CLASS Emiter4
#define EMITER_CLASS_TEMPLATE_PARAMETERS class A1, class A2, class A3, class A4
#define EMITER_PARAMETERS_LIST A1 a1, A2 a2, A3 a3, A4 a4
#define EMITER_PARAMETER_TYPES_LIST A1, A2, A3, A4
#define EMITER_ARGUMENTS_LIST a1, a2, a3, a4

#include "signal.h"

#undef EMITER_CLASS
#undef EMITER_CLASS_TEMPLATE_PARAMETERS
#undef EMITER_PARAMETERS_LIST
#undef EMITER_PARAMETER_TYPES_LIST
#undef EMITER_ARGUMENTS_LIST


#define EMITER_CLASS Emiter5
#define EMITER_CLASS_TEMPLATE_PARAMETERS class A1, class A2, class A3, class A4, class A5
#define EMITER_PARAMETERS_LIST A1 a1, A2 a2, A3 a3, A4 a4, A5 a5
#define EMITER_PARAMETER_TYPES_LIST A1, A2, A3, A4, A5
#define EMITER_ARGUMENTS_LIST a1, a2, a3, a4, a5

#include "signal.h"

#undef EMITER_CLASS
#undef EMITER_CLASS_TEMPLATE_PARAMETERS
#undef EMITER_PARAMETERS_LIST
#undef EMITER_PARAMETER_TYPES_LIST
#undef EMITER_ARGUMENTS_LIST


#define EMITER_CLASS Emiter6
#define EMITER_CLASS_TEMPLATE_PARAMETERS class A1, class A2, class A3, class A4, class A5, class A6
#define EMITER_PARAMETERS_LIST A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6
#define EMITER_PARAMETER_TYPES_LIST A1, A2, A3, A4, A5, A6
#define EMITER_ARGUMENTS_LIST a1, a2, a3, a4, a5, a6

#include "signal.h"

#undef EMITER_CLASS
#undef EMITER_CLASS_TEMPLATE_PARAMETERS
#undef EMITER_PARAMETERS_LIST
#undef EMITER_PARAMETER_TYPES_LIST
#undef EMITER_ARGUMENTS_LIST


#define EMITER_CLASS Emiter7
#define EMITER_CLASS_TEMPLATE_PARAMETERS class A1, class A2, class A3, class A4, class A5, class A6, class A7
#define EMITER_PARAMETERS_LIST A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7
#define EMITER_PARAMETER_TYPES_LIST A1, A2, A3, A4, A5, A6, A7
#define EMITER_ARGUMENTS_LIST a1, a2, a3, a4, a5, a6, a7

#include "signal.h"

#undef EMITER_CLASS
#undef EMITER_CLASS_TEMPLATE_PARAMETERS
#undef EMITER_PARAMETERS_LIST
#undef EMITER_PARAMETER_TYPES_LIST
#undef EMITER_ARGUMENTS_LIST


#define EMITER_CLASS Emiter8
#define EMITER_CLASS_TEMPLATE_PARAMETERS class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8
#define EMITER_PARAMETERS_LIST A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8
#define EMITER_PARAMETER_TYPES_LIST A1, A2, A3, A4, A5, A6, A7, A8
#define EMITER_ARGUMENTS_LIST a1, a2, a3, a4, a5, a6, a7, a8

#include "signal.h"

#undef EMITER_CLASS
#undef EMITER_CLASS_TEMPLATE_PARAMETERS
#undef EMITER_PARAMETERS_LIST
#undef EMITER_PARAMETER_TYPES_LIST
#undef EMITER_ARGUMENTS_LIST


#define EMITER_CLASS Emiter9
#define EMITER_CLASS_TEMPLATE_PARAMETERS class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9
#define EMITER_PARAMETERS_LIST A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9
#define EMITER_PARAMETER_TYPES_LIST A1, A2, A3, A4, A5, A6, A7, A8, A9
#define EMITER_ARGUMENTS_LIST a1, a2, a3, a4, a5, a6, a7, a8, a9

#include "signal.h"

#undef EMITER_CLASS
#undef EMITER_CLASS_TEMPLATE_PARAMETERS
#undef EMITER_PARAMETERS_LIST
#undef EMITER_PARAMETER_TYPES_LIST
#undef EMITER_ARGUMENTS_LIST


#undef EMITER_INSTANTIATE


#else


#ifdef EMITER_INSTANTIATE

namespace Signal {

#ifdef EMITER_CLASS_TEMPLATE_PARAMETERS
#define EMITER_TEMPLATE_SIGNATURE template<EMITER_CLASS_TEMPLATE_PARAMETERS>
#define EMITER_TYPENAME typename
#else
#define EMITER_TEMPLATE_SIGNATURE
#define EMITER_TYPENAME
#endif

EMITER_TEMPLATE_SIGNATURE
	class EMITER_CLASS: public Private::SignalBase
	{
		class ConnectionBase: public Private::ConnectionBase
		{
		  public:
			ConnectionBase (SignalBase* signal, Receiver* receiver):
				Private::ConnectionBase (signal),
				_base_r (receiver)
			{ }

			virtual void call (EMITER_PARAMETERS_LIST) = 0;

			Receiver*
			receiver() const
			{
				return _base_r;
			}

		  private:
			Receiver* _base_r;
		};

		template<class Receiver>
			class Connection: public ConnectionBase
			{
				typedef void (Receiver::*Method)(EMITER_PARAMETERS_LIST);

			  public:
				Connection (SignalBase* signal, Receiver* r, Method m):
					ConnectionBase (signal, r),
					_r (r),
					_m (m)
				{ }

				void
				call (EMITER_PARAMETERS_LIST)
				{
					(_r->*_m)(EMITER_ARGUMENTS_LIST);
				}

				bool
				is (Receiver* r, Method m)
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
				typedef boost::function<void (EMITER_PARAMETER_TYPES_LIST)> Callback;

			  public:
				BoostFunctionConnection (SignalBase* signal, Callback c):
					_c (c)
				{ }

				void
				call (EMITER_PARAMETERS_LIST)
				{
					_c (EMITER_ARGUMENTS_LIST);
				}

			  private:
				Callback _c;
			};

		typedef std::list<ConnectionBase*> Connections;

	  public:
		virtual ~EMITER_CLASS()
		{
			for (EMITER_TYPENAME Connections::iterator i = _connections.begin(); i != _connections.end(); ++i)
			{
				(*i)->receiver()->_connections.remove (*i);
				delete *i;
			}
			_connections.clear();
		}

		template<class Receiver>
			void
			connect (Receiver* receiver, void (Receiver::*method)(EMITER_PARAMETER_TYPES_LIST))
			{
				Connection<Receiver>* connection = new Connection<Receiver> (this, receiver, method);
				_connections.push_back (connection);
				receiver->_connections.push_back (connection);
			}

		template<class Receiver>
			void
			connect (boost::function<void (EMITER_PARAMETER_TYPES_LIST)> const& function)
			{
				Connection<Receiver>* connection = new BoostFunctionConnection<Receiver> (this, function);
				_connections.push_back (connection);
			}

		template<class Receiver>
			void
			disconnect (Receiver* receiver, void (Receiver::*method)(EMITER_PARAMETER_TYPES_LIST))
			{
				Connection<Receiver>* connection;
				for (EMITER_TYPENAME Connections::iterator i = _connections.begin(); i != _connections.end(); ++i)
				{
					if ((connection = dynamic_cast<Connection<Receiver>*> (*i)))
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
		operator() (EMITER_PARAMETERS_LIST)
		{
			for (EMITER_TYPENAME Connections::iterator i = _connections.begin(); i != _connections.end(); ++i)
				(*i)->call (EMITER_ARGUMENTS_LIST);
		}

		EMITER_TYPENAME Connections::size_type
		connections_number() const { return _connections.size(); }

	  protected:
		void
		disconnect (Private::ConnectionBase* connection)
		{
			EMITER_TYPENAME Connections::iterator i = std::find (_connections.begin(), _connections.end(), connection);
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

#undef EMITER_TEMPLATE_SIGNATURE
#undef EMITER_TYPENAME

#endif

#endif

