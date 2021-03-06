//
// Created by lf on 21-6-28.
//

#include <math.h>
#include "TextureRotationUtil.h"
#include "GPUImageRenderer.h"

GPUImageRenderer::GPUImageRenderer(GPUImageFilter *filter) :
    glTextureId(NO_TEXTURE), imageWidth(0), imageHeight(0){
    if(m_Filter != nullptr) {
        delete m_Filter;
    }
    if(filter != nullptr) {
        m_GPUImageInputFilter = new GPUImageInputFilter();
        GPUImageFilterGroup *filterGroup = new GPUImageFilterGroup();
        filterGroup->addFilter((GPUImageFilter *)m_GPUImageInputFilter);
        filterGroup->addFilter(filter);
        m_Filter = (GPUImageFilter *)filterGroup;
    } else {
        m_GPUImageInputFilter = new GPUImageInputFilter();
        m_Filter = (GPUImageFilter *)m_GPUImageInputFilter;
    }
    UpdateMVPMatrix(0, 0, 1.0f, 1.0f);
}

GPUImageRenderer::~GPUImageRenderer() {
    if(m_Filter) {
        delete m_Filter;
        m_Filter = nullptr;
    }
}

void GPUImageRenderer::UpdateMVPMatrix(int angleX, int angleY, float scaleX, float scaleY)
{
    angleX = angleX % 360;
    angleY = angleY % 360;

    //转化为弧度角
    float radiansX = static_cast<float>(MATH_PI / 180.0f * angleX);
    float radiansY = static_cast<float>(MATH_PI / 180.0f * angleY);
    // Projection matrix
    glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
    //video_filter.glm::mat4 Projection = video_filter.glm::frustum(-ratio, ratio, -1.0f, 1.0f, 4.0f, 100.0f);
    //video_filter.glm::mat4 Projection = video_filter.glm::perspective(45.0f,ratio, 0.1f,100.f);

    // View matrix
    glm::mat4 View = glm::lookAt(
            glm::vec3(0, 0, 4), // Camera is at (0,0,1), in World Space
            glm::vec3(0, 0, 0), // and looks at the origin
            glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    // Model matrix
    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::scale(Model, glm::vec3(scaleX, scaleY, 1.0f));
    Model = glm::rotate(Model, radiansX, glm::vec3(1.0f, 0.0f, 0.0f));
    Model = glm::rotate(Model, radiansY, glm::vec3(0.0f, 1.0f, 0.0f));
    Model = glm::translate(Model, glm::vec3(0.0f, 0.0f, 0.0f));

    m_MVPMatrix = Projection * View * Model;
}

void GPUImageRenderer::runOnDraw(const std::function<void()> &T) {
    {
        std::lock_guard <std::mutex> guard(m_Lock);
        m_RunOnDraw.push(std::move(T));
    }
}

void GPUImageRenderer::runOnDrawEnd(const std::function<void()> &T) {
    {
        std::lock_guard <std::mutex> guard(m_Lock);
        m_RunOnDrawEnd.push(std::move(T));
    }
}

void GPUImageRenderer::runAll(std::queue <std::function<void()>> &queue) {
    {
        std::lock_guard <std::mutex> guard(m_Lock);
        while (!queue.empty()) {
            std::function<void()> f = m_RunOnDraw.front();
            f();
            queue.pop();
        }
    }
}

void GPUImageRenderer::onSurfaceCreated() {
    surfaceCreated = true;
    glClearColor(m_BackgroundRed, m_BackgroundGreen, m_BackgroundBlue, 1);
    glDisable(GL_DEPTH_TEST);

    if(m_Filter) {
        m_Filter->ifNeedInit();
    }
}

void GPUImageRenderer::setRenderImage(RenderImage *image) {
    if (imageWidth != image->width) {
        imageWidth = image->width;
        imageHeight = image->height;
        adjustImageScaling();
    }
    if(m_GPUImageInputFilter != nullptr) {
        m_GPUImageInputFilter->setRenderImage(image);
    }
}

float GPUImageRenderer::addDistance(float coordinate, float distance) {
    return coordinate == 0.0f ? distance : 1 - distance;
}

void GPUImageRenderer::adjustImageScaling() {
    if((imageWidth == 0) || (imageHeight == 0)) {
        return;
    }
    int out_w = outputWidth;
    int out_h = outputHeight;
    if (rotation == ROTATION_270 || rotation == ROTATION_90) {
        out_w = outputHeight;
        out_h = outputWidth;
    }

    float ratio1 = 1.0f * out_w / imageWidth;
    float ratio2 = 1.0f * out_h / imageHeight;
    float ratioMax = std::max(ratio1, ratio2);

    int imageWidthNew = std::round(imageWidth * ratioMax);
    int imageHeightNew = std::round(imageHeight * ratioMax);

    float ratioWidth = 1.0f * imageWidthNew / outputWidth;
    float ratioHeight = 1.0f * imageHeightNew / outputHeight;

    TextureRotationUtil::getRotation(glTextureBuffer, NORMAL, false, false);
}

void GPUImageRenderer::onDrawFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if(!surfaceCreated)
        return;
    runAll(m_RunOnDraw);
    if(m_Filter != nullptr) {
        m_Filter->onDraw(glTextureId, glCubeBuffer, glTextureBuffer);
    }
    runAll(m_RunOnDrawEnd);
}

void GPUImageRenderer::onSurfaceChanged(int width, int height) {
    outputWidth = width;
    outputHeight = height;

    glViewport(0, 0, width, height);
    if(m_Filter != nullptr)
        m_Filter->onOutputSizeChanged(width, height);
}

void GPUImageRenderer::setFilter(GPUImageFilter *filter) {
    runOnDraw([this, filter](){
        GPUImageFilter *oldFilter = m_Filter;
        m_Filter = filter;
        if(oldFilter != nullptr) {
            delete oldFilter;
        }
        m_Filter->ifNeedInit();
        glUseProgram(m_Filter->getProgram());
        m_Filter->onOutputSizeChanged(outputWidth, outputHeight);
    });
}

void GPUImageRenderer::setTexture(GLuint texture) {
    glTextureId = texture;
}
