#pragma once


class UIElement
{
public:
	UIElement() = default;
	virtual ~UIElement() = default;
	UIElement(const UIElement&) = default;
	UIElement(UIElement&&) = default;
	UIElement& operator=(const UIElement&) = default;
	UIElement& operator=(UIElement&&) = default;

	virtual void Execute() = 0;
};
