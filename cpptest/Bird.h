#include "Animation.h"

class CBird : public CAnimation
{
public:
	CBird(const std::string& strName):
		CAnimation(strName)
	{
	}

	std::string GetName()
	{
		return m_strName;
	}

	void SetName(std::string& strName)
	{
		m_strName = strName;
	}
};
