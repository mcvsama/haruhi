/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
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
#include <haruhi/utility/qdom_sequence.h>


#define HARUHI_SAVEABLE_PARAMS_STANDARD_METHODS(klass)											\
	typedef klass ThisType;																		\
	klass();																					\
	virtual ~klass() { }																		\
	protected:																					\
	void get_params (Haruhi::BaseParam const**, std::size_t max_entries) const;					\
	public:

#define HARUHI_CONTROLLER_PARAM(name, min, max, deflt, zeroval, denominator)					\
	enum {																						\
		name##Min = min,																		\
		name##Max = max,																		\
		name##ZeroValue = zeroval,																\
		name##Default = deflt,																	\
		name##Denominator = denominator,														\
	};

#define HARUHI_CONTROLLER_PARAM_CONSTRUCT(var, name, shown_decimals)							\
	var (name##Min,																				\
		 name##Max,																				\
		 name##Default,																			\
		 name##ZeroValue,																		\
		 name##Denominator,																		\
		 #name,																					\
		 1.0f * name##Min / name##Denominator,													\
		 1.0f * name##Max / name##Denominator,													\
		 shown_decimals,																		\
		 (name##Max - name##Min) / 400)

#define HARUHI_CONTROLLER_PARAM_CONSTRUCT_EXPLICIT(var, name, smin, smax, sdecimals, step)		\
	var (name##Min,																				\
		 name##Max,																				\
		 name##Default,																			\
		 name##ZeroValue,																		\
		 name##Denominator,																		\
		 #name,																					\
		 smin, smax, sdecimals, step)

#define HARUHI_CONTROLLER_PARAM_ADDITIONAL_ARGS(name, shown_decimals)							\
		 1.0f * name##Min / name##Denominator,													\
		 1.0f * name##Max / name##Denominator,													\
		 shown_decimals,																		\
		 (name##Max - name##Min) / 400

#define HARUHI_DEFINE_SAVEABLE_PARAMS(klass)													\
	void																						\
	Params::klass::get_params (Haruhi::BaseParam const** tab, std::size_t max_entries) const	\
	{																							\
		std::size_t pos = 0;

#define HARUHI_DEFINE_SAVEABLE_PARAM(param)														\
		tab[pos++] = &param;

#define HARUHI_FINISH_SAVEABLE_PARAMS_DEFINITION()												\
		assert (pos == max_entries);															\
	}



namespace Haruhi {

template<class SubClass>
	class SaveableParams: public SaveableState
	{
	  public:
		typedef BaseParam SubClass::* MemberBaseParamPtr;

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
		save_state (QDomElement& parent) const;

		/**
		 * Restore parameter values from XML.
		 */
		void
		load_state (QDomElement const& element);

	  protected:
		/**
		 * Get array of params.
		 * \param	tab Here will be stored array of pointers to params.
		 * \param	max_entries Max number of entries to be stored (length of the tab array).
		 */
		virtual void
		get_params (BaseParam const** tab, std::size_t max_entries) const = 0;
	};


template<class SubClass>
	inline void
	SaveableParams<SubClass>::sanitize()
	{
		BaseParam const** params = reinterpret_cast<BaseParam const**> (alloca (sizeof (BaseParam*) * SubClass::NUM_PARAMS));
		get_params (params, SubClass::NUM_PARAMS);
		for (std::size_t i = 0; i < SubClass::NUM_PARAMS; ++i)
			params[i]->sanitize();
	}


template<class SubClass>
	inline void
	SaveableParams<SubClass>::save_state (QDomElement& parent) const
	{
		BaseParam const** params = reinterpret_cast<BaseParam const**> (alloca (sizeof (BaseParam*) * SubClass::NUM_PARAMS));
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
		for (QDomElement& e: Haruhi::QDomChildElementsSequence (element))
			if (e.tagName() == "parameter")
				map[e.attribute ("name", "")] = e;

		BaseParam const** params = reinterpret_cast<BaseParam const**> (alloca (sizeof (BaseParam*) * SubClass::NUM_PARAMS));
		get_params (params, SubClass::NUM_PARAMS);
		for (std::size_t i = 0; i < SubClass::NUM_PARAMS; ++i)
		{
			Map::iterator param_iter = map.find (params[i]->name());
			if (param_iter != map.end())
				const_cast<BaseParam*> (params[i])->load_state (param_iter->second);
		}
	}

} // namespace Haruhi

#endif

