#pragma once

namespace panda
{

namespace helper
{

	template<typename Enum>
	class Flags
	{
	public:
		using underlying = std::underlying_type_t<Enum>;
		using enum_type = Enum;

		Flags() : m_value(0) {}
		Flags(Enum v) : m_value(static_cast<underlying>(v)) {}
		Flags(underlying v) : m_value(v) {}

		Flags &operator&=(Enum mask) { m_value &= static_cast<underlying>(mask); return *this; }
		Flags &operator|=(Flags v) { m_value |= v.m_value; return *this; }
		Flags &operator|=(Enum v) { m_value |= static_cast<underlying>(v); return *this; }
		Flags &operator^=(Flags v) { m_value ^= v.m_value; return *this; }
		Flags &operator^=(Enum v) { m_value ^= static_cast<underlying>(v); return *this; }

		operator underlying() const { return m_value; }
		operator bool() const { return m_value != 0; }

		bool operator==(Enum v) const { return m_value == static_cast<underlying>(v); }

		Flags operator|(Flags v) const { return Flags(m_value | v.m_value); }
		Flags operator|(Enum v) const { return Flags(m_value | static_cast<underlying>(v)); }
		Flags operator^(Flags v) const { return Flags(m_value ^ v.m_value); }
		Flags operator^(Enum v) const { return Flags(m_value ^ static_cast<underlying>(v)); }
		Flags operator&(underlying mask) const { return Flags(m_value & mask); }
		Flags operator&(Enum v) const { return Flags(m_value & static_cast<underlying>(v)); }
		Flags operator~() const { return Flags(~m_value); }

		bool operator!() const { return !m_value; }

		bool has(Enum v) const 
		{
			const auto val = static_cast<underlying>(v);
			return (m_value & val) == val && (val != 0 || m_value == val); 
		}

	private:
		underlying m_value;
	};

} // namespace helper

} // namespace panda
