#include <string>

class CAnimation
{
public:
	CAnimation(const std::string& strName)
	{
		m_strName = strName;
	}
	virtual ~CAnimation()
	{
	}

public:
	virtual std::string GetName() = 0;
	virtual void SetName(std::string& strName) = 0;

protected:
	std::string m_strName;
};
