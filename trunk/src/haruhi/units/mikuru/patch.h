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

#ifndef HARUHI__UNITS__MIKURU__PATCH_H__INCLUDED
#define HARUHI__UNITS__MIKURU__PATCH_H__INCLUDED

// Standard:
#include <cstddef>
#include <map>
#include <list>

// Haruhi:
#include <haruhi/haruhi.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/widgets/knob.h>


class Mikuru;


namespace MikuruPrivate {

/**
 * This class is responsible for collecting all parameters
 * and serializing it to XML when saving, or updating Mikuru params when loading.
 */
class Patch: public SaveableState
{
	typedef std::map<QString, QDomElement> Parameters;
	typedef std::list<QDomElement> DomElements;

	static const char* VERSION_D_2010_02_20;

  public:
	Patch (Mikuru* mikuru);

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);

  private:
	QString
	to_string (QString const& value) const
	{
		return value;
	}

	QString
	to_string (int value) const
	{
		return QString::number (value);
	}

	QString
	to_string (Haruhi::ControllerParam const& param) const
	{
		return QString::number (param.get());
	}

	/**
	 * Saves parameters as sub-element of element parameters.
	 */
	template<class Value>
		void
		save_parameter (QDomElement& parameters, QString const& name, Value value) const
		{
			QDomElement param = parameters.ownerDocument().createElement (name);
			param.appendChild (parameters.ownerDocument().createTextNode (to_string (value)));
			parameters.appendChild (param);
		}

	/**
	 * Saves parameters like save_parameter adding Knob config. parameters.
	 */
	template<class Value>
		void
		save_parameter (QDomElement& parameters, QString const& name, Value value, Haruhi::ControllerProxy* proxy) const
		{
			QDomElement param = parameters.ownerDocument().createElement (name);
			param.setAttribute ("user-limit-min", proxy->config()->user_limit_min);
			param.setAttribute ("user-limit-max", proxy->config()->user_limit_max);
			param.setAttribute ("curve", proxy->config()->curve);
			param.appendChild (parameters.ownerDocument().createTextNode (to_string (value)));
			parameters.appendChild (param);
		}

	/**
	 * Loads value from given parameters map.
	 * Does nothing if given parameter name is not found.
	 *
	 * \param	parameters
	 * 			Parameters map.
	 * \param	name
	 * 			Element name.
	 * \param	value
	 * 			Reference to value to be set.
	 */
	template<class Value>
		void
		load_parameter (Parameters const& parameters, QString const& name, Value& value) const
		{
			Parameters::const_iterator p = parameters.find (name);
			if (p != parameters.end())
				convert_string (p->second.text(), value);
		}

	/**
	 * Loads value from parameters map and sets up Knob configuration.
	 */
	template<class Value>
		void
		load_parameter (Parameters const& parameters, QString const& name, Value& value, Haruhi::ControllerProxy* proxy) const
		{
			Parameters::const_iterator p = parameters.find (name);
			if (p != parameters.end())
			{
				if (p->second.hasAttribute ("user-limit-min"))
					proxy->config()->user_limit_min = p->second.attribute ("user-limit-min").toInt();
				if (p->second.hasAttribute ("user-limit-max"))
					proxy->config()->user_limit_max = p->second.attribute ("user-limit-max").toInt();
				if (p->second.hasAttribute ("curve"))
					proxy->config()->curve = p->second.attribute ("curve").toFloat();
				convert_string (p->second.text(), value);
				proxy->apply_config();
			}
		}

	void
	convert_string (QString const& string, int& value) const
	{
		value = string.toInt();
	}

	void
	convert_string (QString const& string, unsigned int& value) const
	{
		value = static_cast<unsigned int> (string.toInt());
	}

	void
	convert_string (QString const& string, QString& value) const
	{
		value = string;
	}

	void
	convert_string (QString const& string, Haruhi::ControllerParam& param) const
	{
		param.set (string.toInt());
	}

	/**
	 * Creates map of parameters.
	 */
	void
	create_parameters (QDomElement const& element, Parameters& parameters) const;

  private:
	Mikuru*	_mikuru;
};

} // namespace MikuruPrivate

#endif

