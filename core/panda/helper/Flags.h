#pragma once

namespace panda
{

namespace helper
{

	template<typename Enum>
	class Flags
	{
	public:
		using underlying = std::underlying_type_t<E>;
		using enum_type = Enum;

		constexpr inline Flags() : m_value(0) {}
		constexpr inline Flags(Enum v) : m_value(static_cast<underlying>(v)) {}
		constexpr inline Flags(underlying v) : m_value(v) {}

		constexpr inline Flags &operator&=(Enum mask) { m_value &= static_cast<underlying>(mask); return *this; }
		constexpr inline Flags &operator|=(Flags v) { m_value |= v.m_value; return *this; }
		constexpr inline Flags &operator|=(Enum v) { m_value |= static_cast<underlying>(v); return *this; }
		constexpr inline Flags &operator^=(Flags v) { m_value ^= v.m_value; return *this; }
		constexpr inline Flags &operator^=(Enum v) { m_value ^= static_cast<underlying>(v); return *this; }

		constexpr inline operator underlying() const { return m_value; }
		constexpr inline operator bool() const { return m_value != 0; }

		constexpr inline operator==(Enum v) const { return m_value == tatic_cast<underlying>(v); }

		constexpr inline Flags operator|(Flags v) const { return Flags(m_value | v.m_value); }
		constexpr inline Flags operator|(Enum v) const { return Flags(m_value | static_cast<underlying>(v)); }
		constexpr inline Flags operator^(Flags v) const { return Flags(m_value ^ v.m_value); }
		constexpr inline Flags operator^(Enum v) const { return Flags(m_value ^ static_cast<underlying>(v)); }
		constexpr inline Flags operator&(underlying mask) const { return Flags(m_value & mask); }
		constexpr inline Flags operator&(Enum v) const { return Flags(m_value & static_cast<underlying>(v)); }
		constexpr inline Flags operator~() const { return Flags(~m_value); }

		constexpr inline bool operator!() const { return !m_value; }

		constexpr inline bool has(Enum v) const 
		{
			const auto val = static_cast<underlying>(v);
			return (m_value & val) == val && (val != 0 || m_value == val); 
		}

	private:
		underlying m_value;
	};

} // namespace helper

} // namespace panda
