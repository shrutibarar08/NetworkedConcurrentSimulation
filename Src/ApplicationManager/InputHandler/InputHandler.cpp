#include "InputHandler.h"

#include "ApplicationManager/Clock/SystemClock.h"
#include "Utils/Logger.h"
#include "WindowManager/Components/KeyboardHandler.h"
#include "WindowManager/Components/MouseHandler.h"

InputHandler::InputHandler(CameraController* cameraController)
	: m_CameraController(cameraController)
{}

void InputHandler::AttachWindows(WindowsSystem* windows)
{
	m_WindowsSystem = windows;
}

void InputHandler::AttachCamera(CameraController* controller)
{
	m_CameraController = controller;
}

void InputHandler::HandleInput()
{
	if (KeyboardHandler::IsKeyDown(VK_SPACE))
	{
		m_ThirdPersonView = false;
	}

	if (!m_ThirdPersonView) return;

	float dt = static_cast<float>(SystemClock::GetRunningDeltaTime());
	DirectX::XMVECTOR moveDir = DirectX::XMVectorZero();
	if (KeyboardHandler::IsKeyDown(m_MoveForwardKey))  m_CameraController->MoveForward(dt);
	if (KeyboardHandler::IsKeyDown(m_MoveBackwardKey)) m_CameraController->MoveForward(-dt);
	if (KeyboardHandler::IsKeyDown(m_MoveLeftKey))	   m_CameraController->MoveRight(-dt);
	if (KeyboardHandler::IsKeyDown(m_MoveRightKey))	   m_CameraController->MoveRight(dt);

	if (m_ThirdPersonView) HandleMouseLook(SystemClock::GetRunningDeltaTime());
}

void InputHandler::SetMouseOnScreen(bool val)
{
	if (val)
	{
		MouseHandler::ResetDelta();
		m_ThirdPersonView = true;
		LOG_INFO("Turned On Mouse!");
	}
	else
	{
		m_ThirdPersonView = false;
	};
}

void InputHandler::HandleMouseLook(float deltaTime) const
{
	int dx = 0, dy = 0;
	MouseHandler::GetRawDelta(dx, dy);

	if (dx == 0 && dy == 0) return;
	if (!m_CameraController) return;

	float smoothing = 0.5f; // between 0.0 and 1.0
	static float smoothedDx = 0, smoothedDy = 0;

	smoothedDx = smoothedDx * (1.0f - smoothing) + dx * smoothing;
	smoothedDy = smoothedDy * (1.0f - smoothing) + dy * smoothing;

	float yawDelta = smoothedDx * m_MouseSensitivityX * 0.001f;
	float pitchDelta = smoothedDy * m_MouseSensitivityY * 0.001f;

	m_CameraController->RotateYaw(yawDelta);
	m_CameraController->RotatePitch(pitchDelta);

	MouseHandler::ResetDelta();
}
