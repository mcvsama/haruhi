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

#ifndef HARUHI__LIB__UNIT__PARAM_H__INCLUDED
#define HARUHI__LIB__UNIT__PARAM_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <memory>

// Lib:
#include <boost/format.hpp>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/atomic.h>
#include <haruhi/utility/numeric.h>
#include <haruhi/utility/signal.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/range.h>


namespace Haruhi {
namespace v07 {

/**
 * Defines internal resolution of 'continuous' parameters. Used by _pf operator.
 */
constexpr int64_t kParamFloatDenominator = 1'000'000'000;


namespace param_literals {

/**
 * floats/doubles can't be passed as template parameters, but integers can. We don't need float/doubles for parameters,
 * really, we can use fixed-point values or something alike. _pf returns x * kParamFloatDenominator which can be later
 * divided by kParamFloatDenominator to get floating-point value.
 *
 * Eg. usage: ParamSomething<0.5_pf, 1.0_pf>.
 */
constexpr int64_t operator"" _pf (long double x)
{
	return x * kParamFloatDenominator;
}

} // namespace param_literals


/**
 * UI traits for continuous parameter
 * (parameter name, shown range, format, unit suffix, etc).
 */
class FloatParamUITraits
{
  public:
	// Ctor
	FloatParamUITraits (std::string name, boost::format format, std::string suffix);

  private:
	std::string		_name;
	boost::format	_format;
	std::string		_suffix;
};


/**
 * UI traits for enum-type parameter
 * (list of available enumeration names, parameter name).
 */
class EnumParamUITraits
{
	// TODO
};


/**
 * UI traits for bool-type parameter
 * (essentially the parameter name).
 */
class BoolParamUITraits
{
	// TODO
};


/**
 * User-configurable settings for an continuous parameter.
 */
class FloatParamSettings
{
  public:
	float			curve		= 0.0;
	Range<int64_t>	user_range	= { 0, 0 };
};


// Forward:
template<class pParamValue>
	class GenericFloatParam;

// Forward:
template<class pParamValue>
	class GenericEnumParam;

// Forward:
template<class pParamValue>
	class GenericBoolParam;


/**
 * Float parameter value. Contains only the value and a reference to Settings object.
 * Use for each voice in polyphonic synthesizer.
 *
 * \param	MinimumValue
 *			Minimum allowed value for parameter. Use operator""_pf (eg. 0.5_pf);
 * \param	MaximumValue
 *			Maximum allowed value for parameter. Use operator""_pf.
 * \param	DefaultValue
 *			Default value for the parameter, also used when reset() is called. Use operator""_pf.
 * \param	NeutralValue
 *			"Neutral" value for parameters, needed to correctly apply user-defined sensitivity curve
 *			for the parameter. Use operator""_pf.
 */
template<int64_t MinimumValue, int64_t MaximumValue, int64_t DefaultValue, int64_t NeutralValue>
	class FloatParamValue
	{
	  public:
		typedef int64_t	Value;

	  public:
		// Ctor
		FloatParamValue (std::shared_ptr<FloatParamSettings>);

		// Ctor
		FloatParamValue (GenericFloatParam<FloatParamValue> const&);

		// Ctor
		FloatParamValue (FloatParamValue const&);

		// =
		FloatParamValue const&
		operator= (GenericFloatParam<FloatParamValue> const&);

		// =
		FloatParamValue const&
		operator= (FloatParamValue const&);

		/**
		 * Atomically get parameter's value.
		 *
		 * \threadsafe
		 */
		Value
		internal_get() const noexcept;

		/**
		 * Atomically assign new value to the parameter.
		 *
		 * \threadsafe
		 *		If on_change and on_change_with_value are also threadsafe.
		 */
		void
		internal_set (Value new_value);

	  public:
		/**
		 * Return allowed range for this parameter.
		 */
		static constexpr Range<Value>
		value_range();

		/**
		 * Return default value, set after reset().
		 */
		static constexpr Value
		default_value();

		/**
		 * Return neutral value (used for user-defined curves).
		 */
		static constexpr Value
		neutral_value();

		/**
		 * Sanitize input value - limit it to value_range().
		 */
		static constexpr Value
		sanitize (Value value);

	  public:
		/**
		 * Emitted when parameter has changed:
		 */
		Signal::Emiter<> on_change;

		/**
		 * Emitted after on_change when parameter has changed.
		 * New value is given as parameter.
		 */
		Signal::Emiter<Value> on_change_with_value;

	  private:
		std::atomic<Value>					_value		{ DefaultValue };
		std::atomic<float>					_modulator	{ 0.0 };
		std::shared_ptr<FloatParamSettings>	_settings;

		static_assert (value_range().includes (default_value()), "DefaultValue outside declared range");
		static_assert (value_range().includes (neutral_value()), "NeutralValue outside declared range");
	};


/**
 * Enumerated parameter value. Contains only the value and a reference to Settings object.
 * Use for each voice in polyphonic synthesizer.
 *
 * \param	MinimumValue
 *			Minimum allowed value for parameter.
 * \param	MaximumValue
 *			Maximum allowed value for parameter.
 * \param	DefaultValue
 *			Default value for the parameter, also used when reset() is called. Use operator""_pf.
 */
template<int64_t MinimumValue, int64_t MaximumValue, int64_t DefaultValue>
	class EnumParamValue
	{
	  public:
		typedef int64_t	Value;

	  public:
		// Ctor
		EnumParamValue();

		// Ctor
		EnumParamValue (GenericEnumParam<EnumParamValue> const&);

		// Ctor
		EnumParamValue (EnumParamValue const&);

		// =
		EnumParamValue const&
		operator= (GenericEnumParam<EnumParamValue> const&);

		// =
		EnumParamValue const&
		operator= (EnumParamValue const&);

		/**
		 * Atomically get parameter's value.
		 *
		 * \threadsafe
		 */
		Value
		internal_get() const noexcept;

		/**
		 * Atomically assign new value to the parameter.
		 *
		 * \threadsafe
		 *		If on_change and on_change_with_value are also threadsafe.
		 */
		void
		internal_set (Value new_value);

	  public:
		/**
		 * Return allowed range for this parameter.
		 */
		static constexpr Range<Value>
		value_range();

		/**
		 * Return default value, set after reset().
		 */
		static constexpr Value
		default_value();

		/**
		 * Sanitize input value - limit it to value_range().
		 */
		static constexpr Value
		sanitize (Value value);

	  public:
		/**
		 * Emitted when parameter has changed:
		 */
		Signal::Emiter<> on_change;

		/**
		 * Emitted after on_change when parameter has changed.
		 * New value is given as parameter.
		 */
		Signal::Emiter<Value> on_change_with_value;

	  private:
		std::atomic<Value> _value { DefaultValue };

		static_assert (value_range().includes (default_value()), "DefaultValue outside declared range");
	};


/**
 * Boolean parameter value. Contains only the value and a reference to Settings object.
 * Use for each voice in polyphonic synthesizer.
 *
 * \param	DefaultValue
 *			Default value for the parameter, also used when reset() is called. Use operator""_pf.
 */
template<bool DefaultValue>
	class BoolParamValue
	{
	  public:
		typedef bool Value;

	  public:
		// Ctor
		BoolParamValue();

		// Ctor
		BoolParamValue (GenericBoolParam<BoolParamValue> const&);

		// Ctor
		BoolParamValue (BoolParamValue const&);

		// =
		BoolParamValue const&
		operator= (GenericBoolParam<BoolParamValue> const&);

		// =
		BoolParamValue const&
		operator= (BoolParamValue const&);

		/**
		 * Atomically get parameter's value.
		 *
		 * \threadsafe
		 */
		Value
		internal_get() const noexcept;

		/**
		 * Atomically assign new value to the parameter.
		 *
		 * \threadsafe
		 *		If on_change and on_change_with_value are also threadsafe.
		 */
		void
		internal_set (Value new_value);

	  public:
		/**
		 * Return default value, set after reset().
		 */
		static constexpr Value
		default_value();

		/**
		 * Sanitize input value - limit it to value_range().
		 */
		static constexpr Value
		sanitize (Value value);

	  public:
		/**
		 * Emitted when parameter has changed:
		 */
		Signal::Emiter<> on_change;

		/**
		 * Emitted after on_change when parameter has changed.
		 * New value is given as parameter.
		 */
		Signal::Emiter<Value> on_change_with_value;

	  private:
		std::atomic<Value> _value { DefaultValue };
	};


/**
 * Common base for *Param objects.
 * *Param object is ready to receive values from Event objects. Applies user settings (limits, curve, etc).
 */
class BasicParam: public SaveableState
{
  public:
	/**
	 * Ctor
	 *
	 * \param	identifier
	 *			Unique identifier of the parameter. Used eg. in save-files.
	 */
	explicit
	BasicParam (std::string identifier);

	// Dtor
	virtual ~BasicParam() = default;

	/**
	 * Return parameter unique identifier.
	 */
	std::string const&
	identifier() const noexcept;

	/**
	 * Reset to default value.
	 */
	virtual void
	reset() = 0;

  private:
	std::string	_identifier;
};


/**
 * Monophonic continuous parameter (volume, phase, panorama, amplitude, level, etc).
 *
 * Corresponding UI widget would be a knob or a slider.
 *
 * \param	ParamValue
 *			Concrete ParamValue type.
 */
template<class pParamValue>
	class GenericFloatParam: public BasicParam
	{
		friend pParamValue;

	  public:
		typedef pParamValue	ParamValue;

	  public:
		/**
		 * Ctor
		 *
		 * \param	identifier
		 *			Param's identifier.
		 */
		GenericFloatParam (std::string identifier, FloatParamUITraits ui_traits);

		// Prevent copying
		GenericFloatParam (GenericFloatParam const&) = delete;

		// Prevent copying
		GenericFloatParam&
		operator= (GenericFloatParam const&) = delete;

		/**
		 * Return reference to FloatParamValue for this param.
		 */
		ParamValue const&
		param_value() const;

		/**
		 * Return reference to FloatParamValue for this param.
		 */
		ParamValue&
		param_value();

		/**
		 * Get user-settings for this parameter.
		 */
		FloatParamSettings const&
		settings() const;

		/**
		 * Set new user-settings for this parameter.
		 */
		void
		set_settings (FloatParamSettings const&);

		/*
		 * BasicParam
		 */

		/**
		 * \threadsafe
		 *		If on_change and on_change_with_value are also threadsafe.
		 */
		void
		reset() override;

		/*
		 * SaveableState
		 */

		void
		save_state (QDomElement& parent) const override;

		void
		load_state (QDomElement const& parent) override;

	  private:
		std::shared_ptr<FloatParamSettings>	_settings;
		ParamValue							_param_value;
		FloatParamUITraits					_param_ui_traits;
	};


/**
 * Monophonic enumerated parameter (wave type, modulation index, etc).
 *
 * Corresponding UI widget would be a combo-box, but a knob or a slider with discrete steps is an option, too.
 *
 * \param	ParamValue
 *			Concrete ParamValue type.
 */
template<class pParamValue>
	class GenericEnumParam: public BasicParam
	{
		friend pParamValue;

	  public:
		typedef pParamValue	ParamValue;

	  public:
		/**
		 * Ctor
		 *
		 * \param	identifier
		 *			Param's identifier.
		 */
		GenericEnumParam (std::string identifier, EnumParamUITraits ui_traits);

		// Prevent copying
		GenericEnumParam (GenericEnumParam const&) = delete;

		// Prevent copying
		GenericEnumParam&
		operator= (GenericEnumParam const&) = delete;

		/**
		 * Return reference to EnumParamValue for this param.
		 */
		ParamValue const&
		param_value() const;

		/**
		 * Return reference to EnumParamValue for this param.
		 */
		ParamValue&
		param_value();

		/*
		 * BasicParam
		 */

		/**
		 * \threadsafe
		 *		If on_change and on_change_with_value are also threadsafe.
		 */
		void
		reset() override;

		/*
		 * SaveableState
		 */

		void
		save_state (QDomElement& parent) const override
		{
			// TODO out to param.cc
			parent.appendChild (parent.ownerDocument().createTextNode (QString::number (_param_value.internal_get())));
			// TODO settings
		}

		void
		load_state (QDomElement const& parent) override
		{
			// TODO out to param.cc
			_param_value.internal_set (parent.text().toLongLong());
			// TODO settings
		}

	  private:
		ParamValue _param_value;
	};


/**
 * Monophonic boolean parameter (wave type, modulation index, etc).
 *
 * Corresponding UI widget would be a button.
 *
 * \param	ParamValue
 *			Concrete ParamValue type.
 */
template<class pParamValue>
	class GenericBoolParam: public BasicParam
	{
		friend pParamValue;

	  public:
		typedef pParamValue	ParamValue;

	  public:
		/**
		 * Ctor
		 *
		 * \param	identifier
		 *			Param's identifier.
		 */
		GenericBoolParam (std::string identifier, BoolParamUITraits ui_traits);

		// Prevent copying
		GenericBoolParam (GenericBoolParam const&) = delete;

		// Prevent copying
		GenericBoolParam&
		operator= (GenericBoolParam const&) = delete;

		/**
		 * Return reference to BoolParamValue for this param.
		 */
		ParamValue const&
		param_value() const;

		/**
		 * Return reference to BoolParamValue for this param.
		 */
		ParamValue&
		param_value();

		/*
		 * BasicParam
		 */

		/**
		 * \threadsafe
		 *		If on_change and on_change_with_value are also threadsafe.
		 */
		void
		reset() override;

		/*
		 * SaveableState
		 */

		void
		save_state (QDomElement& parent) const override
		{
			// TODO out to param.cc
			parent.appendChild (parent.ownerDocument().createTextNode (QString::number (!!_param_value.internal_get())));
			// TODO settings
		}

		void
		load_state (QDomElement const& parent) override
		{
			// TODO out to param.cc
			_param_value.internal_set (!!parent.text().toLongLong());
			// TODO settings
		}

	  private:
		ParamValue _param_value;
	};


/*
 * Shorthands
 */


template<int64_t MinimumValue, int64_t MaximumValue, int64_t DefaultValue, int64_t NeutralValue>
	using FloatParam = GenericFloatParam<FloatParamValue<MinimumValue, MaximumValue, DefaultValue, NeutralValue>>;

template<int64_t MinimumValue, int64_t MaximumValue, int64_t DefaultValue>
	using EnumParam = GenericEnumParam<EnumParamValue<MinimumValue, MaximumValue, DefaultValue>>;

template<int64_t DefaultValue>
	using BoolParam = GenericBoolParam<BoolParamValue<DefaultValue>>;


inline
FloatParamUITraits::FloatParamUITraits (std::string name, boost::format format, std::string suffix):
	_name (name),
	_format (format),
	_suffix (suffix)
{ }


template<int64_t M, int64_t X, int64_t D, int64_t N>
	inline
	FloatParamValue<M, X, D, N>::FloatParamValue (std::shared_ptr<FloatParamSettings> settings):
		_settings (settings)
	{ }


template<int64_t M, int64_t X, int64_t D, int64_t N>
	inline
	FloatParamValue<M, X, D, N>::FloatParamValue (GenericFloatParam<FloatParamValue> const& param):
		_value (param._value.load()),
		_modulator (param._modulator.load()),
		_settings (param._settings)
	{ }


template<int64_t M, int64_t X, int64_t D, int64_t N>
	inline
	FloatParamValue<M, X, D, N>::FloatParamValue (FloatParamValue const& other):
		_value (other._value.load()),
		_modulator (other._modulator.load()),
		_settings (other._settings)
	{ }


template<int64_t M, int64_t X, int64_t D, int64_t N>
	inline FloatParamValue<M, X, D, N> const&
	FloatParamValue<M, X, D, N>::operator= (GenericFloatParam<FloatParamValue> const& param)
	{
		_value = param._value.load();
		_modulator = param._modulator.load();
		_settings = param._settings;
		return *this;
	}


template<int64_t M, int64_t X, int64_t D, int64_t N>
	inline FloatParamValue<M, X, D, N> const&
	FloatParamValue<M, X, D, N>::operator= (FloatParamValue const& other)
	{
		_value = other._value.load();
		_modulator = other._modulator.load();
		_settings = other._settings;
		return *this;
	}


template<int64_t M, int64_t X, int64_t D, int64_t N>
	inline auto
	FloatParamValue<M, X, D, N>::internal_get() const noexcept -> Value
	{
		return _value.load (std::memory_order_relaxed);
	}


template<int64_t M, int64_t X, int64_t D, int64_t N>
	inline void
	FloatParamValue<M, X, D, N>::internal_set (Value new_value)
	{
		new_value = sanitize (new_value);
		_value.store (new_value, std::memory_order_relaxed);
		on_change();
		on_change_with_value (new_value);
	}


template<int64_t M, int64_t X, int64_t D, int64_t N>
	constexpr auto
	FloatParamValue<M, X, D, N>::value_range() -> Range<Value>
	{
		return { M, X };
	}


template<int64_t M, int64_t X, int64_t D, int64_t N>
	constexpr auto
	FloatParamValue<M, X, D, N>::default_value() -> Value
	{
		return D;
	}


template<int64_t M, int64_t X, int64_t D, int64_t N>
	constexpr auto
	FloatParamValue<M, X, D, N>::neutral_value() -> Value
	{
		return N;
	}


template<int64_t M, int64_t X, int64_t D, int64_t N>
	constexpr auto
	FloatParamValue<M, X, D, N>::sanitize (Value value) -> Value
	{
		return clamped (value, value_range());
	}


template<int64_t M, int64_t X, int64_t D>
	inline
	EnumParamValue<M, X, D>::EnumParamValue()
	{ }


template<int64_t M, int64_t X, int64_t D>
	inline
	EnumParamValue<M, X, D>::EnumParamValue (GenericEnumParam<EnumParamValue> const& param):
		_value (param._value.load())
	{ }


template<int64_t M, int64_t X, int64_t D>
	inline
	EnumParamValue<M, X, D>::EnumParamValue (EnumParamValue const& other):
		_value (other._value.load())
	{ }


template<int64_t M, int64_t X, int64_t D>
	inline EnumParamValue<M, X, D> const&
	EnumParamValue<M, X, D>::operator= (GenericEnumParam<EnumParamValue> const& param)
	{
		_value = param._value.load();
		return *this;
	}


template<int64_t M, int64_t X, int64_t D>
	inline EnumParamValue<M, X, D> const&
	EnumParamValue<M, X, D>::operator= (EnumParamValue const& other)
	{
		_value = other._value.load();
		return *this;
	}


template<int64_t M, int64_t X, int64_t D>
	inline auto
	EnumParamValue<M, X, D>::internal_get() const noexcept -> Value
	{
		return _value.load (std::memory_order_relaxed);
	}


template<int64_t M, int64_t X, int64_t D>
	inline void
	EnumParamValue<M, X, D>::internal_set (Value new_value)
	{
		new_value = sanitize (new_value);
		_value.store (new_value, std::memory_order_relaxed);
		on_change();
		on_change_with_value (new_value);
	}


template<int64_t M, int64_t X, int64_t D>
	constexpr auto
	EnumParamValue<M, X, D>::value_range() -> Range<Value>
	{
		return { M, X };
	}


template<int64_t M, int64_t X, int64_t D>
	constexpr auto
	EnumParamValue<M, X, D>::default_value() -> Value
	{
		return D;
	}


template<int64_t M, int64_t X, int64_t D>
	constexpr auto
	EnumParamValue<M, X, D>::sanitize (Value value) -> Value
	{
		return clamped (value, value_range());
	}


template<bool D>
	inline
	BoolParamValue<D>::BoolParamValue()
	{ }


template<bool D>
	inline
	BoolParamValue<D>::BoolParamValue (GenericBoolParam<BoolParamValue> const& param):
		_value (param._value.load())
	{ }


template<bool D>
	inline
	BoolParamValue<D>::BoolParamValue (BoolParamValue const& other):
		_value (other._value.load())
	{ }


template<bool D>
	inline BoolParamValue<D> const&
	BoolParamValue<D>::operator= (GenericBoolParam<BoolParamValue> const& param)
	{
		_value = param._value.load();
		return *this;
	}


template<bool D>
	inline BoolParamValue<D> const&
	BoolParamValue<D>::operator= (BoolParamValue const& other)
	{
		_value = other._value.load();
		return *this;
	}


template<bool D>
	inline auto
	BoolParamValue<D>::internal_get() const noexcept -> Value
	{
		return _value.load (std::memory_order_relaxed);
	}


template<bool D>
	inline void
	BoolParamValue<D>::internal_set (Value new_value)
	{
		new_value = sanitize (new_value);
		_value.store (new_value, std::memory_order_relaxed);
		on_change();
		on_change_with_value (new_value);
	}


template<bool D>
	constexpr auto
	BoolParamValue<D>::default_value() -> Value
	{
		return D;
	}


template<bool D>
	constexpr auto
	BoolParamValue<D>::sanitize (Value value) -> Value
	{
		return !!value;
	}


inline
BasicParam::BasicParam (std::string identifier):
	_identifier (identifier)
{ }


inline std::string const&
BasicParam::identifier() const noexcept
{
	return _identifier;
}


template<class P>
	inline
	GenericFloatParam<P>::GenericFloatParam (std::string identifier, FloatParamUITraits ui_traits):
		BasicParam (identifier),
		_settings (std::make_shared<FloatParamSettings>()),
		_param_value (_settings),
		_param_ui_traits (ui_traits)
	{ }


template<class P>
	inline auto
	GenericFloatParam<P>::param_value() const -> ParamValue const&
	{
		return _param_value;
	}


template<class P>
	inline auto
	GenericFloatParam<P>::param_value() -> ParamValue&
	{
		return _param_value;
	}


template<class P>
	inline FloatParamSettings const&
	GenericFloatParam<P>::settings() const
	{
		return *_settings.get();
	}


template<class P>
	inline void
	GenericFloatParam<P>::set_settings (FloatParamSettings const& settings)
	{
		_settings = std::make_shared<FloatParamSettings> (settings);
	}


template<class P>
	inline void
	GenericFloatParam<P>::reset()
	{
		_param_value.internal_set (ParamValue::default_value());
	}


template<class P>
	inline void
	GenericFloatParam<P>::save_state (QDomElement& parent) const
	{
		parent.appendChild (parent.ownerDocument().createTextNode (QString::number (_param_value.internal_get())));
		// TODO settings
	}


template<class P>
	inline void
	GenericFloatParam<P>::load_state (QDomElement const& parent)
	{
		_param_value.internal_set (parent.text().toLongLong());
		// TODO settings
	}

} // namespace v07
} // namespace Haruhi

#endif

