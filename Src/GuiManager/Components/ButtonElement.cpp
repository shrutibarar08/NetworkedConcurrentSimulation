#include "ButtonElement.h"

ButtonElement::~ButtonElement()
{
	m_CallbackFns.clear();
}

void ButtonElement::AddCallbackFn(std::function<void()> fn)
{
	m_CallbackFns.emplace_back(fn);
}

void ButtonElement::Execute()
{
}
