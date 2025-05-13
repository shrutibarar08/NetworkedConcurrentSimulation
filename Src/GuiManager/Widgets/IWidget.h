#pragma once
#include <string>


class IWidget
{
public:
	IWidget() = default;
	virtual ~IWidget() = default;

	IWidget(const IWidget&) = delete;
	IWidget(IWidget&&) = delete;
	IWidget& operator=(const IWidget&) = delete;
	IWidget& operator=(IWidget&&) = delete;

	virtual bool Init() { return true; }
	virtual void RenderMenu() {};
	virtual void RenderAsSystemItem() {}
	virtual void RenderPopups(){}
	virtual std::string MenuName() const = 0;

	bool IsNeededMenu() const { return m_NeededMenu; }
	void NeedMenu(bool val) { m_NeededMenu = val; }

protected:
	bool m_NeededMenu{ false };
};
