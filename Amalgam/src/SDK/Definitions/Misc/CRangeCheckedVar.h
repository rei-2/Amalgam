#pragma once

template< class T, int minValue, int maxValue, int startValue >
class CRangeCheckedVar
{
public:

	inline CRangeCheckedVar()
	{
		m_Val = startValue;
	}

	inline CRangeCheckedVar(const T& value)
	{
		*this = value;
	}

	T GetRaw() const
	{
		return m_Val;
	}

	inline void Clamp()
	{
		if (m_Val < minValue)
			m_Val = minValue;

		else if (m_Val > maxValue)
			m_Val = maxValue;
	}

	inline operator const T& () const
	{
		return m_Val;
	}

	inline CRangeCheckedVar<T, minValue, maxValue, startValue>& operator=(const T& value)
	{
		//RangeCheck(value, minValue, maxValue);
		m_Val = value;
		return *this;
	}

	inline CRangeCheckedVar<T, minValue, maxValue, startValue>& operator+=(const T& value)
	{
		return (*this = m_Val + value);
	}

	inline CRangeCheckedVar<T, minValue, maxValue, startValue>& operator-=(const T& value)
	{
		return (*this = m_Val - value);
	}

	inline CRangeCheckedVar<T, minValue, maxValue, startValue>& operator*=(const T& value)
	{
		return (*this = m_Val * value);
	}

	inline CRangeCheckedVar<T, minValue, maxValue, startValue>& operator/=(const T& value)
	{
		return (*this = m_Val / value);
	}

private:
	T m_Val;
};