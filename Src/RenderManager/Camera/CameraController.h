#pragma once

#include <windows.h>
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <memory>


class CameraController
{
public:
    // Constructor initializes camera with given ID and name
    CameraController(int id, const std::string& name);

	void SetPosition(float x, float y, float z);
	void SetOrientation(float yaw, float pitch);
	void SetLens(float fovDegrees, float aspect, float nearZ, float farZ);
    void SetOrthogonalBounds(float left, float right, float bottom, float top, float nearZ, float farZ);
    void GetLens(float& fovDegrees, float& aspect, float& nearZ, float& farZ);
    void GetOrientation(float& yaw, float& pitch);

    void Log() const;

	int GetID();
    std::string GetName();

    DirectX::XMFLOAT3 GetPosition();
    DirectX::XMMATRIX GetViewMatrix();
    DirectX::XMMATRIX GetProjectionMatrix();
    DirectX::XMMATRIX GetOrthogonalMatrix();

private:
    // CameraController state
    int m_id;
    std::string m_name;
    DirectX::XMFLOAT3 m_position;
    float m_yaw, m_pitch;         // Orientation angles in radians
    DirectX::XMFLOAT4X4 m_viewMatrix;      // Cached view matrix
    bool m_viewDirty;             // Flag to recompute view matrix
    float m_fov, m_aspect, m_nearZ, m_farZ;
    DirectX::XMFLOAT4X4 m_projMatrix;      // Cached projection matrix
    bool m_projDirty;             // Flag to recompute projection matrix

    SRWLOCK m_lock;              // Protects camera data for thread safety
    float m_left, m_right, m_bottom, m_top;
};

//-----------------------------------------------------------------------------
// CameraManager Class - Manages multiple CameraController instances (thread-safe).
// Uses SRWLOCK for protecting camera list and a Win32 Mutex for ID generation.
//-----------------------------------------------------------------------------
class CameraManager
{
public:
    CameraManager();
    ~CameraManager();

    int AddCamera(const std::string& name);
	bool RemoveCamera(int id);
    void SetActiveCamera(int id);

    CameraController* GetCamera(int id);
    CameraController* GetCameraByName(const std::string& name);
    CameraController* GetActiveCamera();
    // Perform lazy updates on all cameras' matrices (call from main thread)
    void UpdateAllCameras();

private:
    std::vector<std::unique_ptr<CameraController>> m_cameras;
    CameraController* m_activeCamera;   // Currently active camera
    int m_nextID;             // For generating unique camera IDs
    HANDLE m_idMutex;         // Win32 mutex for protecting m_nextID
    SRWLOCK m_lock;           // Protects m_cameras and m_activeCamera
};
