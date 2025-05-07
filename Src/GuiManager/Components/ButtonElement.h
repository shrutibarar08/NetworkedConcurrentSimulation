#pragma once
#include <functional>
#include <unordered_map>
#include "UIElement.h"


class ButtonElement final: public UIElement
{
public:
	ButtonElement() = default;
	~ButtonElement() override;

	void AddCallbackFn(std::function<void()> fn);
	void Execute() override;

private:
	std::vector<std::function<void()>> m_CallbackFns;
};
