
/*
 * Copyright (c) 2023 - 2025 the ThorVG project. All rights reserved.

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <algorithm>
#include "tvgMath.h"
#include "tvgWgRenderData.h"
#include "tvgWgShaderTypes.h"

//***********************************************************************
// WgMeshData
//***********************************************************************

void WgMeshData::update(const WgVertexBuffer& vertexBuffer)
{
    assert(vertexBuffer.count > 2);
    // setup vertex data
    vbuffer.reserve(vertexBuffer.count);
    vbuffer.count = vertexBuffer.count;
    memcpy(vbuffer.data, vertexBuffer.data, sizeof(vertexBuffer.data[0])*vertexBuffer.count);
    // setup tex coords data
    tbuffer.clear();
}


void WgMeshData::update(const WgIndexedVertexBuffer& vertexBufferInd)
{
    assert(vertexBufferInd.vcount > 2);
    // setup vertex data
    vbuffer.reserve(vertexBufferInd.vcount);
    vbuffer.count = vertexBufferInd.vcount;
    memcpy(vbuffer.data, vertexBufferInd.vbuff, sizeof(vertexBufferInd.vbuff[0])*vertexBufferInd.vcount);
    // setup tex coords data
    tbuffer.clear();
    // copy index data
    ibuffer.reserve(vertexBufferInd.icount);
    ibuffer.count = vertexBufferInd.icount;
    memcpy(ibuffer.data, vertexBufferInd.ibuff, sizeof(vertexBufferInd.ibuff[0])*vertexBufferInd.icount);
};


void WgMeshData::bbox(const Point pmin, const Point pmax)
{
    const float data[] = {pmin.x, pmin.y, pmax.x, pmin.y, pmax.x, pmax.y, pmin.x, pmax.y};
    // setup vertex data
    vbuffer.reserve(4);
    vbuffer.count = 4;
    memcpy(vbuffer.data, data, sizeof(data));
    // setup tex coords data
    tbuffer.clear();
}


void WgMeshData::imageBox(float w, float h)
{
    const float vdata[] = {0.0f, 0.0f, w, 0.0f, w, h, 0.0f, h};
    const float tdata[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
    const uint32_t idata[] = {0, 1, 2, 0, 2, 3};
    // setup vertex data
    vbuffer.reserve(4);
    vbuffer.count = 4;
    memcpy(vbuffer.data, vdata, sizeof(vdata));
    // setup tex coords data
    tbuffer.reserve(4);
    tbuffer.count = 4;
    memcpy(tbuffer.data, tdata, sizeof(tdata));
    // setup indexes data
    ibuffer.reserve(6);
    ibuffer.count = 6;
    memcpy(ibuffer.data, idata, sizeof(idata));
}


void WgMeshData::blitBox()
{
    const float vdata[] = {-1.0f, +1.0f, +1.0f, +1.0f, +1.0f, -1.0f, -1.0f, -1.0f};
    const float tdata[] = {+0.0f, +0.0f, +1.0f, +0.0f, +1.0f, +1.0f, +0.0f, +1.0f};
    const uint32_t idata[] = { 0, 1, 2, 0, 2, 3 };
    // setup vertex data
    vbuffer.reserve(4);
    vbuffer.count = 4;
    memcpy(vbuffer.data, vdata, sizeof(vdata));
    // setup tex coords data
    tbuffer.reserve(4);
    tbuffer.count = 4;
    memcpy(tbuffer.data, tdata, sizeof(tdata));
    // setup indexes data
    ibuffer.reserve(6);
    ibuffer.count = 6;
    memcpy(ibuffer.data, idata, sizeof(idata));
}

//***********************************************************************
// WgMeshDataGroup
//***********************************************************************

void WgMeshDataGroup::append(const WgVertexBuffer& vertexBuffer)
{
    assert(vertexBuffer.count >= 3);
    meshes.push(new WgMeshData());
    meshes.last()->update(vertexBuffer);
}


void WgMeshDataGroup::append(const WgIndexedVertexBuffer& vertexBufferInd)
{
    assert(vertexBufferInd.vcount >= 3);
    meshes.push(new WgMeshData());
    meshes.last()->update(vertexBufferInd);
}


void WgMeshDataGroup::append(const Point pmin, const Point pmax)
{
    meshes.push(new WgMeshData());
    meshes.last()->bbox(pmin, pmax);
}


void WgMeshDataGroup::release()
{
    ARRAY_FOREACH(p, meshes) delete *p;
    meshes.clear();
};


//***********************************************************************
// WgImageData
//***********************************************************************

void WgImageData::update(WgContext& context, const RenderSurface* surface)
{
    // get appropriate texture format from color space
    WGPUTextureFormat texFormat = WGPUTextureFormat_BGRA8Unorm;
    if (surface->cs == ColorSpace::ABGR8888S)
        texFormat = WGPUTextureFormat_RGBA8Unorm;
    if (surface->cs == ColorSpace::Grayscale8)
        texFormat = WGPUTextureFormat_R8Unorm;
    // allocate new texture handle
    bool texHandleChanged = context.allocateTexture(texture, surface->w, surface->h, texFormat, surface->data);
    // update texture view of texture handle was changed
    if (texHandleChanged) {
        context.releaseTextureView(textureView);
        textureView = context.createTextureView(texture);
        // update bind group
        context.layouts.releaseBindGroup(bindGroup);
        bindGroup = context.layouts.createBindGroupTexSampled(context.samplerLinearRepeat, textureView);
    }
};


void WgImageData::update(WgContext& context, const Fill* fill)
{
    // compute gradient data
    WgShaderTypeGradientData gradientData;
    gradientData.update(fill);
    // allocate new texture handle
    bool texHandleChanged = context.allocateTexture(texture, WG_TEXTURE_GRADIENT_SIZE, 1, WGPUTextureFormat_RGBA8Unorm, gradientData.data);
    // update texture view of texture handle was changed
    if (texHandleChanged) {
        context.releaseTextureView(textureView);
        textureView = context.createTextureView(texture);
        // get sampler by spread type
        WGPUSampler sampler = context.samplerLinearClamp;
        if (fill->spread() == FillSpread::Reflect) sampler = context.samplerLinearMirror;
        if (fill->spread() == FillSpread::Repeat) sampler = context.samplerLinearRepeat;
        // update bind group
        context.layouts.releaseBindGroup(bindGroup);
        bindGroup = context.layouts.createBindGroupTexSampled(sampler, textureView);
    }
};


void WgImageData::release(WgContext& context)
{
    context.layouts.releaseBindGroup(bindGroup);
    context.releaseTextureView(textureView);
    context.releaseTexture(texture);
};

//***********************************************************************
// WgRenderSettings
//***********************************************************************

void WgRenderSettings::update(WgContext& context, const tvg::Matrix& transform, tvg::ColorSpace cs, uint8_t opacity)
{
    settings.transform.update(transform);
    settings.options.update(cs, opacity);
}

void WgRenderSettings::update(WgContext& context, const Fill* fill)
{
    assert(fill);
    settings.gradient.update(fill);
    gradientData.update(context, fill);
    // get gradient rasterisation settings
    rasterType = WgRenderRasterType::Gradient;
    if (fill->type() == Type::LinearGradient)
        fillType = WgRenderSettingsType::Linear;
    else if (fill->type() == Type::RadialGradient)
        fillType = WgRenderSettingsType::Radial;
    skip = false;
};


void WgRenderSettings::update(WgContext& context, const RenderColor& c)
{
    settings.color.update(c);
    rasterType = WgRenderRasterType::Solid;
    fillType = WgRenderSettingsType::Solid;
    skip = (c.a == 0);
};


void WgRenderSettings::release(WgContext& context)
{
    gradientData.release(context);
};

//***********************************************************************
// WgRenderDataPaint
//***********************************************************************

void WgRenderDataPaint::release(WgContext& context)
{
    clips.clear();
};


void WgRenderDataPaint::updateClips(tvg::Array<tvg::RenderData> &clips) {
    this->clips.clear();
    ARRAY_FOREACH(p, clips) {
        this->clips.push((WgRenderDataPaint*)(*p));
    }
}

//***********************************************************************
// WgRenderDataShape
//***********************************************************************

void WgRenderDataShape::appendShape(const WgVertexBuffer& vertexBuffer)
{
    if (vertexBuffer.count < 3) return;
    Point pmin{}, pmax{};
    vertexBuffer.getMinMax(pmin, pmax);
    meshGroupShapes.append(vertexBuffer);
    meshGroupShapesBBox.append(pmin, pmax);
    updateBBox(pmin, pmax);
}


void WgRenderDataShape::appendStroke(const WgIndexedVertexBuffer& vertexBufferInd)
{
    if (vertexBufferInd.vcount < 3) return;
    Point pmin{}, pmax{};
    vertexBufferInd.getMinMax(pmin, pmax);
    meshGroupStrokes.append(vertexBufferInd);
    meshGroupStrokesBBox.append(pmin, pmax);
    updateBBox(pmin, pmax);
}


void WgRenderDataShape::updateBBox(Point pmin, Point pmax)
{
    pMin.x = std::min(pMin.x, pmin.x);
    pMin.y = std::min(pMin.y, pmin.y);
    pMax.x = std::max(pMax.x, pmax.x);
    pMax.y = std::max(pMax.y, pmax.y);
}


void WgRenderDataShape::updateAABB(const Matrix& tr) {
    auto p0 = Point{pMin.x, pMin.y} * tr;
    auto p1 = Point{pMax.x, pMin.y} * tr;
    auto p2 = Point{pMin.x, pMax.y} * tr;
    auto p3 = Point{pMax.x, pMax.y} * tr;
    aabb.min = {std::min(std::min(p0.x, p1.x), std::min(p2.x, p3.x)), std::min(std::min(p0.y, p1.y), std::min(p2.y, p3.y))};
    aabb.max = {std::max(std::max(p0.x, p1.x), std::max(p2.x, p3.x)), std::max(std::max(p0.y, p1.y), std::max(p2.y, p3.y))};
}


void WgRenderDataShape::updateMeshes(const RenderShape &rshape, const Matrix& tr, WgGeometryBufferPool* pool)
{
    releaseMeshes();
    strokeFirst = rshape.strokeFirst();

    // get object scale
    float scale = std::max(std::min(length(Point{tr.e11 + tr.e12,tr.e21 + tr.e22}), 8.0f), 1.0f);

    // path decoded vertex buffer
    auto pbuff = pool->reqVertexBuffer(scale);

    pbuff->decodePath(rshape, true, [&](const WgVertexBuffer& path_buff) {
        appendShape(path_buff);
        if ((rshape.stroke) && (rshape.stroke->width > 0)) proceedStrokes(rshape.stroke, path_buff, pool);
    }, rshape.trimpath());

    // update shapes bbox (with empty path handling)
    if ((this->meshGroupShapesBBox.meshes.count > 0 ) ||
        (this->meshGroupStrokesBBox.meshes.count > 0)) {
        updateAABB(tr);
    } else aabb = {{0, 0}, {0, 0}};
    meshDataBBox.bbox(pMin, pMax);

    pool->retVertexBuffer(pbuff);
}


void WgRenderDataShape::proceedStrokes(const RenderStroke* rstroke, const WgVertexBuffer& buff, WgGeometryBufferPool* pool)
{
    assert(rstroke);
    auto strokesGenerator = pool->reqIndexedVertexBuffer(buff.scale);
    if (rstroke->dash.length < DASH_PATTERN_THRESHOLD) strokesGenerator->appendStrokes(buff, rstroke);
    else strokesGenerator->appendStrokesDashed(buff, rstroke);

    appendStroke(*strokesGenerator);

    pool->retIndexedVertexBuffer(strokesGenerator);
}


void WgRenderDataShape::releaseMeshes()
{
    meshGroupStrokesBBox.release();
    meshGroupStrokes.release();
    meshGroupShapesBBox.release();
    meshGroupShapes.release();
    pMin = {FLT_MAX, FLT_MAX};
    pMax = {0.0f, 0.0f};
    aabb = {{0, 0}, {0, 0}};
    clips.clear();
}


void WgRenderDataShape::release(WgContext& context)
{
    releaseMeshes();
    renderSettingsStroke.release(context);
    renderSettingsShape.release(context);
    WgRenderDataPaint::release(context);
};

//***********************************************************************
// WgRenderDataShapePool
//***********************************************************************

WgRenderDataShape* WgRenderDataShapePool::allocate(WgContext& context)
{
    WgRenderDataShape* renderData{};
    if (mPool.count > 0) {
        renderData = mPool.last();
        mPool.pop();
    } else {
        renderData = new WgRenderDataShape();
        mList.push(renderData);
    }
    return renderData;
}


void WgRenderDataShapePool::free(WgContext& context, WgRenderDataShape* renderData)
{
    renderData->meshGroupShapes.release();
    renderData->meshGroupShapesBBox.release();
    renderData->meshGroupStrokes.release();
    renderData->meshGroupStrokesBBox.release();
    renderData->clips.clear();
    mPool.push(renderData);
}


void WgRenderDataShapePool::release(WgContext& context)
{
    ARRAY_FOREACH(p, mList) {
        (*p)->release(context);
        delete(*p);
    }
    mPool.clear();
    mList.clear();
}

//***********************************************************************
// WgRenderDataPicture
//***********************************************************************

void WgRenderDataPicture::updateSurface(WgContext& context, const RenderSurface* surface)
{
    // upoate mesh data
    meshData.imageBox(surface->w, surface->h);
    // update texture data
    imageData.update(context, surface);
}


void WgRenderDataPicture::release(WgContext& context)
{
    renderSettings.release(context);
    imageData.release(context);
    WgRenderDataPaint::release(context);
}

//***********************************************************************
// WgRenderDataPicturePool
//***********************************************************************

WgRenderDataPicture* WgRenderDataPicturePool::allocate(WgContext& context)
{
    WgRenderDataPicture* renderData{};
    if (mPool.count > 0) {
        renderData = mPool.last();
        mPool.pop();
    } else {
        renderData = new WgRenderDataPicture();
        mList.push(renderData);
    }
    return renderData;
}


void WgRenderDataPicturePool::free(WgContext& context, WgRenderDataPicture* renderData)
{
    renderData->clips.clear();
    mPool.push(renderData);
}


void WgRenderDataPicturePool::release(WgContext& context)
{
    ARRAY_FOREACH(p, mList) {
        (*p)->release(context);
        delete(*p);
    }
    mPool.clear();
    mList.clear();
}

//***********************************************************************
// WgRenderDataGaussian
//***********************************************************************

void WgRenderDataViewport::update(WgContext& context, const RenderRegion& region) {
    WgShaderTypeVec4f viewport;
    viewport.update(region);
    bool bufferViewportChanged = context.allocateBufferUniform(bufferViewport, &viewport, sizeof(viewport));
    if (bufferViewportChanged) {
        context.layouts.releaseBindGroup(bindGroupViewport);
        bindGroupViewport = context.layouts.createBindGroupBuffer1Un(bufferViewport);
    }
}


void WgRenderDataViewport::release(WgContext& context) {
    context.releaseBuffer(bufferViewport);
    context.layouts.releaseBindGroup(bindGroupViewport);
}

//***********************************************************************
// WgRenderDataViewportPool
//***********************************************************************

WgRenderDataViewport* WgRenderDataViewportPool::allocate(WgContext& context)
{
    WgRenderDataViewport* renderData{};
    if (mPool.count > 0) {
        renderData = mPool.last();
        mPool.pop();
    } else {
        renderData = new WgRenderDataViewport();
        mList.push(renderData);
    }
    return renderData;
}


void WgRenderDataViewportPool::free(WgContext& context, WgRenderDataViewport* renderData)
{
    if (renderData) mPool.push(renderData);
}


void WgRenderDataViewportPool::release(WgContext& context)
{
    ARRAY_FOREACH(p, mList) {
        (*p)->release(context);
        delete(*p);
    }
    mPool.clear();
    mList.clear();
}

//***********************************************************************
// WgRenderDataEffectParams
//***********************************************************************

void WgRenderDataEffectParams::update(WgContext& context, WgShaderTypeEffectParams& effectParams)
{
    if (context.allocateBufferUniform(bufferParams, &effectParams.params, sizeof(effectParams.params))) {
        context.layouts.releaseBindGroup(bindGroupParams);
        bindGroupParams = context.layouts.createBindGroupBuffer1Un(bufferParams);
    }
}


void WgRenderDataEffectParams::update(WgContext& context, RenderEffectGaussianBlur* gaussian, const Matrix& transform)
{
    assert(gaussian);
    WgShaderTypeEffectParams effectParams;
    if (!effectParams.update(gaussian, transform)) return;
    update(context, effectParams);
    level = int(WG_GAUSSIAN_MAX_LEVEL * ((gaussian->quality - 1) * 0.01f)) + 1;
    extend = effectParams.extend;
}


void WgRenderDataEffectParams::update(WgContext& context, RenderEffectDropShadow* dropShadow, const Matrix& transform)
{
    assert(dropShadow);
    WgShaderTypeEffectParams effectParams;
    if (!effectParams.update(dropShadow, transform)) return;
    update(context, effectParams);
    level = int(WG_GAUSSIAN_MAX_LEVEL * ((dropShadow->quality - 1) * 0.01f)) + 1;
    extend = effectParams.extend;
    offset = effectParams.offset;
}


void WgRenderDataEffectParams::update(WgContext& context, RenderEffectFill* fill)
{
    assert(fill);
    WgShaderTypeEffectParams effectParams;
    if (!effectParams.update(fill)) return;
    update(context, effectParams);
}


void WgRenderDataEffectParams::update(WgContext& context, RenderEffectTint* tint)
{
    assert(tint);
    WgShaderTypeEffectParams effectParams;
    if (!effectParams.update(tint)) return;
    update(context, effectParams);
}


void WgRenderDataEffectParams::update(WgContext& context, RenderEffectTritone* tritone)
{
    assert(tritone);
    WgShaderTypeEffectParams effectParams;
    if (!effectParams.update(tritone)) return;
    update(context, effectParams);
}


void WgRenderDataEffectParams::release(WgContext& context)
{
    context.releaseBuffer(bufferParams);
    context.layouts.releaseBindGroup(bindGroupParams);
}

//***********************************************************************
// WgRenderDataColorsPool
//***********************************************************************

WgRenderDataEffectParams* WgRenderDataEffectParamsPool::allocate(WgContext& context)
{
    WgRenderDataEffectParams* renderData{};
    if (mPool.count > 0) {
        renderData = mPool.last();
        mPool.pop();
    } else {
        renderData = new WgRenderDataEffectParams();
        mList.push(renderData);
    }
    return renderData;
}


void WgRenderDataEffectParamsPool::free(WgContext& context, WgRenderDataEffectParams* renderData)
{
    if (renderData) mPool.push(renderData);
}


void WgRenderDataEffectParamsPool::release(WgContext& context)
{
    ARRAY_FOREACH(p, mList) {
        (*p)->release(context);
        delete(*p);
    }
    mPool.clear();
    mList.clear();
}

//***********************************************************************
// WgStageBufferGeometry
//***********************************************************************

void WgStageBufferGeometry::append(WgMeshData* meshData)
{
    assert(meshData);
    uint32_t vsize = meshData->vbuffer.count * sizeof(meshData->vbuffer[0]);
    uint32_t tsize = meshData->tbuffer.count * sizeof(meshData->tbuffer[0]);
    uint32_t isize = meshData->ibuffer.count * sizeof(meshData->ibuffer[0]);
    vmaxcount = std::max(vmaxcount, meshData->vbuffer.count);
    // append vertex data
    if (vbuffer.reserved < vbuffer.count + vsize)
        vbuffer.grow(std::max(vsize, vbuffer.reserved));
    if (meshData->vbuffer.count > 0) {
        meshData->voffset = vbuffer.count;
        memcpy(vbuffer.data + vbuffer.count, meshData->vbuffer.data, vsize);
        vbuffer.count += vsize;
    }
    // append tex coords data
    if (vbuffer.reserved < vbuffer.count + tsize)
        vbuffer.grow(std::max(tsize, vbuffer.reserved));
    if (meshData->tbuffer.count > 0) {
        meshData->toffset = vbuffer.count;
        memcpy(vbuffer.data + vbuffer.count, meshData->tbuffer.data, tsize);
        vbuffer.count += tsize;
    }
    // append index data
    if (ibuffer.reserved < ibuffer.count + isize)
        ibuffer.grow(std::max(isize, ibuffer.reserved));
    if (meshData->ibuffer.count > 0) {
        meshData->ioffset = ibuffer.count;
        memcpy(ibuffer.data + ibuffer.count, meshData->ibuffer.data, isize);
        ibuffer.count += isize;
    }
}


void WgStageBufferGeometry::append(WgMeshDataGroup* meshDataGroup)
{
    ARRAY_FOREACH(p, meshDataGroup->meshes) append(*p);
}


void WgStageBufferGeometry::append(WgRenderDataShape* renderDataShape)
{
    append(&renderDataShape->meshGroupShapes);
    append(&renderDataShape->meshGroupShapesBBox);
    append(&renderDataShape->meshGroupStrokes);
    append(&renderDataShape->meshGroupStrokesBBox);
    append(&renderDataShape->meshDataBBox);
}


void WgStageBufferGeometry::append(WgRenderDataPicture* renderDataPicture)
{
    append(&renderDataPicture->meshData);
}


void WgStageBufferGeometry::release(WgContext& context)
{
    context.releaseBuffer(vbuffer_gpu);
    context.releaseBuffer(ibuffer_gpu);
}


void WgStageBufferGeometry::clear()
{
    vbuffer.clear();
    ibuffer.clear();
    vmaxcount = 0;
}


void WgStageBufferGeometry::flush(WgContext& context) 
{
    context.allocateBufferVertex(vbuffer_gpu, (float *)vbuffer.data, vbuffer.count);
    context.allocateBufferIndex(ibuffer_gpu, (uint32_t *)ibuffer.data, ibuffer.count);
    context.allocateBufferIndexFan(vmaxcount);
}
