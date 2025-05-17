#pragma once

using ID = unsigned int;

class IEntity
{
public:
	IEntity()
		: m_Id(++ID_PROVIDER)
	{}

	ID GetId() const { return m_Id; }
	

protected:
	inline static unsigned int ID_PROVIDER{ 0 };

private:
	ID m_Id;
};
