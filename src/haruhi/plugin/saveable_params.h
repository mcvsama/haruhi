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

#ifndef HARUHI__PLUGIN__SAVEABLE_PARAMS_H__INCLUDED
#define HARUHI__PLUGIN__SAVEABLE_PARAMS_H__INCLUDED

// Standard:
#include <cstddef>
#include <map>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/lib/param.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/qdom.h>


#define HARUHI_SAVEABLE_PARAMS_STANDARD_METHODS(klass)											\
	typedef klass ThisType;																		\
	klass();																					\
	virtual ~klass() = default;																	\
	protected:																					\
	void get_params (Haruhi::v06::BasicParam const**, std::size_t max_entries) const;			\
	public:

#define HARUHI_CONTROLLER_PARAM(name, min, max, deflt, centerval, denominator)					\
	enum {																						\
		name##Min = min,																		\
		name##Max = max,																		\
		name##CenterValue = centerval,															\
		name##Default = deflt,																	\
		name##Denominator = denominator,														\
	};

#define HARUHI_CONTROLLER_PARAM_CONSTRUCT(var, name, shown_decimals)							\
	var ({ name##Min, name##Max },																\
		 name##Default,																			\
		 name##CenterValue,																		\
		 name##Denominator,																		\
		 #name,																					\
		 { 1.0f * name##Min / name##Denominator, 1.0f * name##Max / name##Denominator },		\
		 shown_decimals,																		\
		 (name##Max - name##Min) / 400)

#define HARUHI_CONTROLLER_PARAM_CONSTRUCT_EXPLICIT(var, name, shown_range, sdecimals, step)		\
	var ({ name##Min, name##Max },																\
		 name##Default,																			\
		 name##CenterValue,																		\
		 name##Denominator,																		\
		 #name,																					\
		 shown_range, sdecimals, step)

#define HARUHI_CONTROLLER_PARAM_ADDITIONAL_ARGS(name, shown_decimals)							\
		{ 1.0f * name##Min / name##Denominator, 1.0f * name##Max / name##Denominator },			\
		shown_decimals,																			\
		(name##Max - name##Min) / 400

#define HARUHI_DEFINE_SAVEABLE_PARAMS(klass)													\
	void																						\
	Params::klass::get_params (Haruhi::v06::BasicParam const** tab, std::size_t max_entries) const \
	{																							\
		std::size_t pos = 0;

#define HARUHI_DEFINE_SAVEABLE_PARAM(param)														\
		tab[pos++] = &param;

#define HARUHI_FINISH_SAVEABLE_PARAMS_DEFINITION()												\
		assert_function (pos == max_entries);													\
	}



namespace Haruhi {

template<class SubClass>
	class SaveableParams: public SaveableState
	{
	  public:
		typedef v06::BasicParam SubClass::* MemberBaseParamPtr;

	  public:
		/**
		 * Sanitize params - call .sanitize() on each param.
		 */
		void
		sanitize();

		/**
		 * Dump/marshal all parameters into XML node.
		 */
		void
		save_state (QDomElement& parent) const override;

		/**
		 * Restore parameter values from XML.
		 */
		void
		load_state (QDomElement const& element) override;

	  protected:
		/**
		 * Get array of params.
		 * \param	tab Here will be stored array of pointers to params.
		 * \param	max_entries Max number of entries to be stored (length of the tab array).
		 */
		virtual void
		get_params (v06::BasicParam const** tab, std::size_t max_entries) const = 0;
	};


template<class SubClass>
	inline void
	SaveableParams<SubClass>::sanitize()
	{
		v06::BasicParam** params = reinterpret_cast<v06::BasicParam**> (alloca (sizeof (v06::BasicParam*) * SubClass::NUM_PARAMS));
		get_params (params, SubClass::NUM_PARAMS);
		for (std::size_t i = 0; i < SubClass::NUM_PARAMS; ++i)
			params[i]->sanitize();
	}


template<class SubClass>
	inline void
	SaveableParams<SubClass>::save_state (QDomElement& parent) const
	{
		v06::BasicParam const** params = reinterpret_cast<v06::BasicParam const**> (alloca (sizeof (v06::BasicParam*) * SubClass::NUM_PARAMS));
		get_params (params, SubClass::NUM_PARAMS);
		for (std::size_t i = 0; i < SubClass::NUM_PARAMS; ++i)
		{
			QDomElement param_el = parent.ownerDocument().createElement ("parameter");
			param_el.setAttribute ("name", params[i]->name());
			params[i]->save_state (param_el);
			parent.appendChild (param_el);
		}
	}


template<class SubClass>
	inline void
	SaveableParams<SubClass>::load_state (QDomElement const& element)
	{
		typedef std::map<QString, QDomElement> Map;

		Map map;
		for (QDomElement& e: element)
			if (e.tagName() == "parameter")
				map[e.attribute ("name", "")] = e;

		v06::BasicParam const** params = reinterpret_cast<v06::BasicParam const**> (alloca (sizeof (v06::BasicParam*) * SubClass::NUM_PARAMS));
		get_params (params, SubClass::NUM_PARAMS);
		for (std::size_t i = 0; i < SubClass::NUM_PARAMS; ++i)
		{
			Map::iterator param_iter = map.find (params[i]->name());
			if (param_iter != map.end())
				const_cast<v06::BasicParam*> (params[i])->load_state (param_iter->second);
		}
	}

} // namespace Haruhi

#endif

