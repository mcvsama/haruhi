/* vim:ts=4
 *
 * Copyleft 2008…2010  Michał Gawron
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


class Signal0;
template<class> class Signal1;
template<class, class> class Signal2;
template<class, class, class> class Signal3;
template<class, class, class, class> class Signal4;
template<class, class, class, class, class> class Signal5;
template<class, class, class, class, class, class> class Signal6;
template<class, class, class, class, class, class, class> class Signal7;
template<class, class, class, class, class, class, class, class> class Signal8;
template<class, class, class, class, class, class, class, class, class> class Signal9;

class Receiver
{
	friend class Signal0;
	template<class> friend class Signal1;
	template<class, class> friend class Signal2;
	template<class, class, class> friend class Signal3;
	template<class, class, class, class> friend class Signal4;
	template<class, class, class, class, class> friend class Signal5;
	template<class, class, class, class, class, class> friend class Signal6;
	template<class, class, class, class, class, class, class> friend class Signal7;
	template<class, class, class, class, class, class, class, class> friend class Signal8;
	template<class, class, class, class, class, class, class, class, class> friend class Signal9;

  public:
	virtual ~Receiver()
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


#define SIGNAL_INSTANTIATE


#define SIGNAL_CLASS Signal0
#define SIGNAL_PARAMETERS_LIST
#define SIGNAL_PARAMETER_TYPES_LIST
#define SIGNAL_ARGUMENTS_LIST

#include "signal.h"

#undef SIGNAL_CLASS
#undef SIGNAL_CLASS_TEMPLATE_PARAMETERS
#undef SIGNAL_PARAMETERS_LIST
#undef SIGNAL_PARAMETER_TYPES_LIST
#undef SIGNAL_ARGUMENTS_LIST


#define SIGNAL_CLASS Signal1
#define SIGNAL_CLASS_TEMPLATE_PARAMETERS class A1
#define SIGNAL_PARAMETERS_LIST A1 a1
#define SIGNAL_PARAMETER_TYPES_LIST A1
#define SIGNAL_ARGUMENTS_LIST a1

#include "signal.h"

#undef SIGNAL_CLASS
#undef SIGNAL_CLASS_TEMPLATE_PARAMETERS
#undef SIGNAL_PARAMETERS_LIST
#undef SIGNAL_PARAMETER_TYPES_LIST
#undef SIGNAL_ARGUMENTS_LIST


#define SIGNAL_CLASS Signal2
#define SIGNAL_CLASS_TEMPLATE_PARAMETERS class A1, class A2
#define SIGNAL_PARAMETERS_LIST A1 a1, A2 a2
#define SIGNAL_PARAMETER_TYPES_LIST A1, A2
#define SIGNAL_ARGUMENTS_LIST a1, a2

#include "signal.h"

#undef SIGNAL_CLASS
#undef SIGNAL_CLASS_TEMPLATE_PARAMETERS
#undef SIGNAL_PARAMETERS_LIST
#undef SIGNAL_PARAMETER_TYPES_LIST
#undef SIGNAL_ARGUMENTS_LIST


#define SIGNAL_CLASS Signal3
#define SIGNAL_CLASS_TEMPLATE_PARAMETERS class A1, class A2, class A3
#define SIGNAL_PARAMETERS_LIST A1 a1, A2 a2, A3 a3
#define SIGNAL_PARAMETER_TYPES_LIST A1, A2, A3
#define SIGNAL_ARGUMENTS_LIST a1, a2, a3

#include "signal.h"

#undef SIGNAL_CLASS
#undef SIGNAL_CLASS_TEMPLATE_PARAMETERS
#undef SIGNAL_PARAMETERS_LIST
#undef SIGNAL_PARAMETER_TYPES_LIST
#undef SIGNAL_ARGUMENTS_LIST


#define SIGNAL_CLASS Signal4
#define SIGNAL_CLASS_TEMPLATE_PARAMETERS class A1, class A2, class A3, class A4
#define SIGNAL_PARAMETERS_LIST A1 a1, A2 a2, A3 a3, A4 a4
#define SIGNAL_PARAMETER_TYPES_LIST A1, A2, A3, A4
#define SIGNAL_ARGUMENTS_LIST a1, a2, a3, a4

#include "signal.h"

#undef SIGNAL_CLASS
#undef SIGNAL_CLASS_TEMPLATE_PARAMETERS
#undef SIGNAL_PARAMETERS_LIST
#undef SIGNAL_PARAMETER_TYPES_LIST
#undef SIGNAL_ARGUMENTS_LIST


#define SIGNAL_CLASS Signal5
#define SIGNAL_CLASS_TEMPLATE_PARAMETERS class A1, class A2, class A3, class A4, class A5
#define SIGNAL_PARAMETERS_LIST A1 a1, A2 a2, A3 a3, A4 a4, A5 a5
#define SIGNAL_PARAMETER_TYPES_LIST A1, A2, A3, A4, A5
#define SIGNAL_ARGUMENTS_LIST a1, a2, a3, a4, a5

#include "signal.h"

#undef SIGNAL_CLASS
#undef SIGNAL_CLASS_TEMPLATE_PARAMETERS
#undef SIGNAL_PARAMETERS_LIST
#undef SIGNAL_PARAMETER_TYPES_LIST
#undef SIGNAL_ARGUMENTS_LIST


#define SIGNAL_CLASS Signal6
#define SIGNAL_CLASS_TEMPLATE_PARAMETERS class A1, class A2, class A3, class A4, class A5, class A6
#define SIGNAL_PARAMETERS_LIST A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6
#define SIGNAL_PARAMETER_TYPES_LIST A1, A2, A3, A4, A5, A6
#define SIGNAL_ARGUMENTS_LIST a1, a2, a3, a4, a5, a6

#include "signal.h"

#undef SIGNAL_CLASS
#undef SIGNAL_CLASS_TEMPLATE_PARAMETERS
#undef SIGNAL_PARAMETERS_LIST
#undef SIGNAL_PARAMETER_TYPES_LIST
#undef SIGNAL_ARGUMENTS_LIST


#define SIGNAL_CLASS Signal7
#define SIGNAL_CLASS_TEMPLATE_PARAMETERS class A1, class A2, class A3, class A4, class A5, class A6, class A7
#define SIGNAL_PARAMETERS_LIST A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7
#define SIGNAL_PARAMETER_TYPES_LIST A1, A2, A3, A4, A5, A6, A7
#define SIGNAL_ARGUMENTS_LIST a1, a2, a3, a4, a5, a6, a7

#include "signal.h"

#undef SIGNAL_CLASS
#undef SIGNAL_CLASS_TEMPLATE_PARAMETERS
#undef SIGNAL_PARAMETERS_LIST
#undef SIGNAL_PARAMETER_TYPES_LIST
#undef SIGNAL_ARGUMENTS_LIST


#define SIGNAL_CLASS Signal8
#define SIGNAL_CLASS_TEMPLATE_PARAMETERS class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8
#define SIGNAL_PARAMETERS_LIST A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8
#define SIGNAL_PARAMETER_TYPES_LIST A1, A2, A3, A4, A5, A6, A7, A8
#define SIGNAL_ARGUMENTS_LIST a1, a2, a3, a4, a5, a6, a7, a8

#include "signal.h"

#undef SIGNAL_CLASS
#undef SIGNAL_CLASS_TEMPLATE_PARAMETERS
#undef SIGNAL_PARAMETERS_LIST
#undef SIGNAL_PARAMETER_TYPES_LIST
#undef SIGNAL_ARGUMENTS_LIST


#define SIGNAL_CLASS Signal9
#define SIGNAL_CLASS_TEMPLATE_PARAMETERS class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9
#define SIGNAL_PARAMETERS_LIST A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9
#define SIGNAL_PARAMETER_TYPES_LIST A1, A2, A3, A4, A5, A6, A7, A8, A9
#define SIGNAL_ARGUMENTS_LIST a1, a2, a3, a4, a5, a6, a7, a8, a9

#include "signal.h"

#undef SIGNAL_CLASS
#undef SIGNAL_CLASS_TEMPLATE_PARAMETERS
#undef SIGNAL_PARAMETERS_LIST
#undef SIGNAL_PARAMETER_TYPES_LIST
#undef SIGNAL_ARGUMENTS_LIST


#undef SIGNAL_INSTANTIATE


#else


#ifdef SIGNAL_INSTANTIATE

namespace Signal {

#ifdef SIGNAL_CLASS_TEMPLATE_PARAMETERS
#define SIGNAL_TEMPLATE_SIGNATURE template<SIGNAL_CLASS_TEMPLATE_PARAMETERS>
#define SIGNAL_TYPENAME typename
#else
#define SIGNAL_TEMPLATE_SIGNATURE
#define SIGNAL_TYPENAME
#endif

SIGNAL_TEMPLATE_SIGNATURE
	class SIGNAL_CLASS: public Private::SignalBase
	{
		class ConnectionBase: public Private::ConnectionBase
		{
		  public:
			ConnectionBase (SignalBase* signal, Receiver* receiver):
				Private::ConnectionBase (signal),
				_base_r (receiver)
			{ }

			virtual void call (SIGNAL_PARAMETERS_LIST) = 0;

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
				typedef void (Receiver::*Method)(SIGNAL_PARAMETERS_LIST);

			  public:
				Connection (SignalBase* signal, Receiver* r, Method m):
					ConnectionBase (signal, r),
					_r (r),
					_m (m)
				{ }

				void
				call (SIGNAL_PARAMETERS_LIST)
				{
					(_r->*_m)(SIGNAL_ARGUMENTS_LIST);
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

		typedef std::list<ConnectionBase*> Connections;

	  public:
		virtual ~SIGNAL_CLASS()
		{
			for (SIGNAL_TYPENAME Connections::iterator i = _connections.begin(); i != _connections.end(); ++i)
			{
				(*i)->receiver()->_connections.remove (*i);
				delete *i;
			}
			_connections.clear();
		}

		template<class Receiver>
			void
			connect (Receiver* receiver, void (Receiver::*method)(SIGNAL_PARAMETER_TYPES_LIST))
			{
				Connection<Receiver>* connection = new Connection<Receiver> (this, receiver, method);
				_connections.push_back (connection);
				receiver->_connections.push_back (connection);
			}

		template<class Receiver>
			void
			disconnect (Receiver* receiver, void (Receiver::*method)(SIGNAL_PARAMETER_TYPES_LIST))
			{
				Connection<Receiver>* connection;
				for (SIGNAL_TYPENAME Connections::iterator i = _connections.begin(); i != _connections.end(); ++i)
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
		operator() (SIGNAL_PARAMETERS_LIST)
		{
			for (SIGNAL_TYPENAME Connections::iterator i = _connections.begin(); i != _connections.end(); ++i)
				(*i)->call (SIGNAL_ARGUMENTS_LIST);
		}

	  protected:
		void
		disconnect (Private::ConnectionBase* connection)
		{
			SIGNAL_TYPENAME Connections::iterator i = std::find (_connections.begin(), _connections.end(), connection);
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

#undef SIGNAL_TEMPLATE_SIGNATURE
#undef SIGNAL_TYPENAME

#endif

#endif

