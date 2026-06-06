#pragma once

template <typename T>
class Dvar
{
	private:
	T value;
	T prevValue;

    public:
    Dvar() = default;
    Dvar(const T& initialValue) : value(initialValue), prevValue(initialValue) {}
    bool IsModified() 
    {
        bool isModified = prevValue != value;
        if (isModified) 
        {
            prevValue = value;
        }
        return isModified;
    }

    operator T() const { return value; }

    // Assignment from T — stores old value before updating
    Dvar& operator=(const T& newValue)
    {
        prevValue = value;
        value = newValue;
        return *this;
    }

    // Assignment from another Dvar
    Dvar& operator=(const Dvar& other)
    {
        prevValue = value;
        value = other.value;
        return *this;
    }

    T GetValue() 
    {
        return value;
    }

    T GetPreviousValue()
    {
        return prevValue;
    }

};

