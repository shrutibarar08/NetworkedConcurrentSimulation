#include "CameraController.h"

#include <cmath>

#include "Utils/Logger.h"

CameraController::CameraController(int id, const std::string& name)
    : m_id(id), m_name(name),
    m_position(0.0f, 0.0f, 0.0f),
    m_yaw(0.0f), m_pitch(0.0f),
    m_fov(0.0f), m_aspect(0.0f), m_nearZ(0.0f), m_farZ(0.0f),
    m_viewDirty(true), m_projDirty(true)
{
    // Initialize default lens (45-degree FOV, 4:3 aspect, near=0.1, far=1000)
    m_fov = XMConvertToRadians(45.0f);
    m_aspect = 4.0f / 3.0f;
    m_nearZ = 0.1f;
    m_farZ = 1000.0f;
    // Initialize view/projection matrices to identity
    XMStoreFloat4x4(&m_viewMatrix, XMMatrixIdentity());
    XMStoreFloat4x4(&m_projMatrix, XMMatrixIdentity());
    // Initialize the SRWLOCK for thread safety
    InitializeSRWLock(&m_lock);
}

int CameraController::GetID()
{
    AcquireSRWLockShared(&m_lock);
    int id = m_id;
    ReleaseSRWLockShared(&m_lock);
    return id;
}

std::string CameraController::GetName()
{
    AcquireSRWLockShared(&m_lock);
    std::string name = m_name;
    ReleaseSRWLockShared(&m_lock);
    return name;
}

void CameraController::SetPosition(float x, float y, float z)
{
    AcquireSRWLockExclusive(&m_lock);
    m_position = XMFLOAT3(x, y, z);
    m_viewDirty = true;
    ReleaseSRWLockExclusive(&m_lock);
}

XMFLOAT3 CameraController::GetPosition()
{
    AcquireSRWLockShared(&m_lock);
    XMFLOAT3 pos = m_position;
    ReleaseSRWLockShared(&m_lock);
    return pos;
}

void CameraController::SetOrientation(float yaw, float pitch)
{
    AcquireSRWLockExclusive(&m_lock);
    m_yaw = yaw;
    m_pitch = pitch;
    m_viewDirty = true;
    ReleaseSRWLockExclusive(&m_lock);
}

void CameraController::GetOrientation(float& yaw, float& pitch)
{
    AcquireSRWLockShared(&m_lock);
    yaw = m_yaw;
    pitch = m_pitch;
    ReleaseSRWLockShared(&m_lock);
}

void CameraController::SetLens(float fovDegrees, float aspect, float nearZ, float farZ)
{
    AcquireSRWLockExclusive(&m_lock);
    m_fov = XMConvertToRadians(fovDegrees);
    m_aspect = aspect;
    m_nearZ = nearZ;
    m_farZ = farZ;
    m_projDirty = true;
    ReleaseSRWLockExclusive(&m_lock);
}

void CameraController::SetOrthogonalBounds(float left, float right, float bottom, float top, float nearZ, float farZ)
{
    AcquireSRWLockExclusive(&m_lock);
    m_left = left;
    m_right = right;
    m_bottom = bottom;
    m_top = top;
    m_nearZ = nearZ;
    m_farZ = farZ;
    m_projDirty = true;
    ReleaseSRWLockExclusive(&m_lock);
}

void CameraController::GetLens(float& fovDegrees, float& aspect, float& nearZ, float& farZ)
{
    AcquireSRWLockShared(&m_lock);
    fovDegrees = XMConvertToDegrees(m_fov);
    aspect = m_aspect;
    nearZ = m_nearZ;
    farZ = m_farZ;
    ReleaseSRWLockShared(&m_lock);
}

XMMATRIX CameraController::GetViewMatrix()
{
    // First check without exclusive lock for performance
    AcquireSRWLockShared(&m_lock);
    if (!m_viewDirty) {
        // View matrix is up-to-date
        XMMATRIX view = XMLoadFloat4x4(&m_viewMatrix);
        ReleaseSRWLockShared(&m_lock);
        return view;
    }
    ReleaseSRWLockShared(&m_lock);

    // View is dirty: recompute under exclusive lock
    AcquireSRWLockExclusive(&m_lock);
    if (m_viewDirty) {
        // Recompute the view matrix
        XMVECTOR posVec = XMLoadFloat3(&m_position);
        // Calculate forward vector from yaw & pitch
        float cosPitch = cosf(m_pitch);
        float sinPitch = sinf(m_pitch);
        float cosYaw = cosf(m_yaw);
        float sinYaw = sinf(m_yaw);
        XMVECTOR forward = XMVectorSet(
            sinYaw * cosPitch,
            sinPitch,
            cosPitch * cosYaw,
            0.0f
        );
        XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        XMMATRIX viewMat = XMMatrixLookToLH(posVec, forward, up);
        XMStoreFloat4x4(&m_viewMatrix, viewMat);
        m_viewDirty = false;
    }
    // Load the view matrix to return it
    XMMATRIX result = XMLoadFloat4x4(&m_viewMatrix);
    ReleaseSRWLockExclusive(&m_lock);
    return result;
}

XMMATRIX CameraController::GetProjectionMatrix()
{
    AcquireSRWLockShared(&m_lock);
    if (!m_projDirty) {
        // Projection matrix is up-to-date
        XMMATRIX proj = XMLoadFloat4x4(&m_projMatrix);
        ReleaseSRWLockShared(&m_lock);
        return proj;
    }
    ReleaseSRWLockShared(&m_lock);

    AcquireSRWLockExclusive(&m_lock);
    if (m_projDirty) {
        // Recompute the projection matrix
        XMMATRIX projMat = XMMatrixPerspectiveFovLH(m_fov, m_aspect, m_nearZ, m_farZ);
        XMStoreFloat4x4(&m_projMatrix, projMat);
        m_projDirty = false;
    }
    XMMATRIX result = XMLoadFloat4x4(&m_projMatrix);
    ReleaseSRWLockExclusive(&m_lock);
    return result;
}

XMMATRIX CameraController::GetOrthogonalMatrix()
{
    AcquireSRWLockShared(&m_lock);
    if (!m_projDirty) {
        // Projection matrix is up-to-date
        XMMATRIX ortho = XMLoadFloat4x4(&m_projMatrix);
        ReleaseSRWLockShared(&m_lock);
        return ortho;
    }
    ReleaseSRWLockShared(&m_lock);

    AcquireSRWLockExclusive(&m_lock);
    if (m_projDirty) {
        // Recompute the orthographic projection matrix
        XMMATRIX orthoMat = XMMatrixOrthographicOffCenterLH(m_left,
            m_right,
            m_bottom,
            m_top,
            m_nearZ, m_farZ);
        XMStoreFloat4x4(&m_projMatrix, orthoMat);
        m_projDirty = false;
    }
    XMMATRIX result = XMLoadFloat4x4(&m_projMatrix);
    ReleaseSRWLockExclusive(&m_lock);
    return result;
}

CameraManager::CameraManager()
    : m_activeCamera(nullptr), m_nextID(1)
{
    InitializeSRWLock(&m_lock);
    // Create a mutex for unique ID generation
    m_idMutex = CreateMutex(NULL, FALSE, NULL);
    // In production code, check for CreateMutex failure here
}

CameraManager::~CameraManager()
{
    if (m_idMutex) {
        CloseHandle(m_idMutex);
    }
}

int CameraManager::AddCamera(const std::string& name)
{
    // Generate unique ID with mutex protection
    WaitForSingleObject(m_idMutex, INFINITE);
    int id = m_nextID++;
    ReleaseMutex(m_idMutex);

    // Create and add the new CameraController
    std::unique_ptr<CameraController> cam = std::make_unique<CameraController>(id, name);

    AcquireSRWLockExclusive(&m_lock);
    m_cameras.push_back(std::move(cam));
    // If no active camera, set this as active
    if (!m_activeCamera) {
        m_activeCamera = m_cameras.back().get();
    }
    ReleaseSRWLockExclusive(&m_lock);

    LOG_INFO("Added Camera Component: " + name);

    return id;
}

bool CameraManager::RemoveCamera(int id)
{
    AcquireSRWLockExclusive(&m_lock);
    for (auto it = m_cameras.begin(); it != m_cameras.end(); ++it) {
        if ((*it)->GetID() == id)
        {
            if (m_activeCamera == it->get()) m_activeCamera = nullptr;
            m_cameras.erase(it);
            ReleaseSRWLockExclusive(&m_lock);
            return true;
        }
    }
    ReleaseSRWLockExclusive(&m_lock);
    return false; // Not found
}

CameraController* CameraManager::GetCamera(int id)
{
    AcquireSRWLockShared(&m_lock);
    for (const auto& camPtr : m_cameras) {
        if (camPtr->GetID() == id) {
            CameraController* cam = camPtr.get();
            ReleaseSRWLockShared(&m_lock);
            return cam;
        }
    }
    ReleaseSRWLockShared(&m_lock);
    return nullptr;
}

CameraController* CameraManager::GetCameraByName(const std::string& name)
{
    AcquireSRWLockShared(&m_lock);
    for (const auto& camPtr : m_cameras) {
        if (camPtr->GetName() == name) {
            CameraController* cam = camPtr.get();
            ReleaseSRWLockShared(&m_lock);
            return cam;
        }
    }
    ReleaseSRWLockShared(&m_lock);
    return nullptr;
}

void CameraManager::SetActiveCamera(int id)
{
    AcquireSRWLockExclusive(&m_lock);
    for (const auto& camPtr : m_cameras) {
        if (camPtr->GetID() == id) {
            m_activeCamera = camPtr.get();
            break;
        }
    }
    ReleaseSRWLockExclusive(&m_lock);
}

CameraController* CameraManager::GetActiveCamera()
{
    AcquireSRWLockShared(&m_lock);
    CameraController* cam = m_activeCamera;
    ReleaseSRWLockShared(&m_lock);
    return cam;
}

void CameraManager::UpdateAllCameras()
{
    AcquireSRWLockShared(&m_lock);
    for (const auto& camPtr : m_cameras) {
        // Trigger view/projection updates if needed
        camPtr->GetViewMatrix();
        camPtr->GetProjectionMatrix();
    }
    ReleaseSRWLockShared(&m_lock);
}
