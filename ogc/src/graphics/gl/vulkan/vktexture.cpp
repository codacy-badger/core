#ifdef __VULKAN__

#include "graphics/graphics.h"
#include "graphics/gl/vulkan.h"
#include "graphics/objects/texture/texture.h"
#include "graphics/objects/texture/texturelist.h"
#include "graphics/objects/render/commandlist.h"
using namespace oi::gc;
using namespace oi;

bool Texture::initData(bool isOwned) {

	owned = isOwned;

	VkGraphics &graphics = g->getExtension();

	bool useStencil = false, useDepth = false;

	if (info.format == TextureFormat::Depth) {

		useDepth = true;

		std::vector<VkTextureFormat> priorities = { VkTextureFormat::D32S8, VkTextureFormat::D24S8, VkTextureFormat::D32, VkTextureFormat::D16 };

		for (VkTextureFormat &f : priorities) {
			VkFormatProperties fprop;
			vkGetPhysicalDeviceFormatProperties(graphics.pdevice, (VkFormat) f.getValue().value, &fprop);

			if (fprop.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
				info.format = f.getName();
				break;
			}
		}

		if (info.format == TextureFormat::Depth)
			return Log::throwError<VkTexture, 0x0>("Couldn't get depth texture; no optimal format available");

		useStencil = info.format.getIndex() > TextureFormat::D32;
	}

	VkTextureFormat format = info.format.getName();
	VkFormat format_inter = (VkFormat) format.getValue().value;

	VkTextureUsage usage = info.usage.getName();

	if(isOwned){
	
		//Create image

		VkImageCreateInfo imageInfo;
		memset(&imageInfo, 0, sizeof(imageInfo));

		useDepth = info.format.getValue() >= TextureFormat::D16 && info.format.getValue() <= TextureFormat::D32S8;

		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = format_inter;
		imageInfo.extent = { info.res.x, info.res.y, 1 };
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.mipLevels = info.mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.usage = (info.usage == TextureUsage::Image ? VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT : (useDepth ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		vkCheck<0x4, VkTexture>(vkCreateImage(graphics.device, &imageInfo, vkAllocator, &ext.resource), "Couldn't create image");
		vkName(graphics, ext.resource, VK_OBJECT_TYPE_IMAGE, getName());

		//Allocate memory (TODO: by GraphicsExt)

		VkMemoryAllocateInfo memoryInfo;
		memset(&memoryInfo, 0, sizeof(memoryInfo));

		VkMemoryRequirements requirements;
		vkGetImageMemoryRequirements(graphics.device, ext.resource, &requirements);

		memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryInfo.allocationSize = requirements.size;

		uint32_t memoryIndex = u32_MAX;

		for (uint32_t i = 0; i < graphics.pmemory.memoryTypeCount; ++i)
			if ((requirements.memoryTypeBits & (1 << i)) && (graphics.pmemory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0) {
				memoryIndex = i;
				break;
			}

		if (memoryIndex == u32_MAX)
			Log::throwError<VkTexture, 0x1>(String("Couldn't find a valid memory type for a VkTexture: ") + getName());

		memoryInfo.memoryTypeIndex = memoryIndex;

		vkCheck<0x5, VkTexture>(vkAllocateMemory(graphics.device, &memoryInfo, vkAllocator, &ext.memory), "Couldn't allocate memory");
		vkCheck<0x6, VkTexture>(vkBindImageMemory(graphics.device, ext.resource, ext.memory, 0), String("Couldn't bind memory to texture ") + getName());

	}
	
	//Create image view

	VkImageViewCreateInfo viewInfo;
	memset(&viewInfo, 0, sizeof(viewInfo));

	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = ext.resource;
	viewInfo.format = format_inter;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.subresourceRange.aspectMask = useDepth ? (useStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0) | VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.levelCount = info.mipLevels;
	viewInfo.subresourceRange.layerCount = 1;

	vkCheck<0x7, VkTexture>(vkCreateImageView(graphics.device, &viewInfo, vkAllocator, &ext.view), "Couldn't create image view");
	vkName(graphics, ext.view, VK_OBJECT_TYPE_IMAGE_VIEW, getName() + " view");

	//Prepare texture for update

	if (info.dat.size() != 0U) {

		if (info.dat.size() != info.res.x * info.res.y * Graphics::getFormatSize(info.format))
			return Log::throwError<VkTexture, 0x2>("The buffer was of incorrect size");

		flush(Vec2u(), info.res);

	}

	if (info.parent != nullptr)
		info.handle = info.parent->alloc(this);

	Log::println(String("Successfully created a VkTexture with format ") + info.format.getName() + " and size " + info.res);
	return true;
}

Texture::~Texture() {
	
	if (info.dat.size() != 0)
		free(info.dat.addr());

	if(info.parent != nullptr)
		info.parent->dealloc(this);

	if (g != nullptr) {

		VkGraphics &graphics = g->getExtension();

		vkDestroyImageView(graphics.device, ext.view, vkAllocator);
		
		if(owned){
			vkFreeMemory(graphics.device, ext.memory, vkAllocator);
			vkDestroyImage(graphics.device, ext.resource, vkAllocator);
		}

	}
}

void Texture::push() {

	if (!shouldStage())
		return;

	GraphicsExt &graphics = g->getExtension();

	//Prepare data for transfer

	Vec2u changedLength = info.changedEnd - info.changedStart;

	u32 stride = getStride();
	u32 size = changedLength.x * changedLength.y * stride;
	bool fullTexture = changedLength == info.res;

	Buffer dat = fullTexture ? info.dat : Buffer(size);

	if (!fullTexture) {

		if (changedLength.x == info.res.x)		//Copy rows (fast)
			memcpy(dat.addr(), info.dat.addr() + info.changedStart.y * changedLength.x * stride, size);
		else {

			//Copy rows (slow)
			for(u32 i = 0; i < changedLength.y; ++i)
				memcpy(
					dat.addr() + i * changedLength.x * stride,
					info.dat.addr() + ((info.changedStart.y + i) * info.res.x + info.changedStart.x) * stride,
					changedLength.x * stride
				);

		}

	}

	//Construct staging buffer with data

	GBufferExt gbext;
	gbext.resource.resize(1);

	VkBufferCreateInfo stagingInfo;
	memset(&stagingInfo, 0, sizeof(stagingInfo));

	stagingInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingInfo.size = size;
	stagingInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	stagingInfo.queueFamilyIndexCount = 1;
	stagingInfo.pQueueFamilyIndices = &graphics.queueFamilyIndex;

	vkCheck<0x8, VkTexture>(vkCreateBuffer(graphics.device, &stagingInfo, vkAllocator, gbext.resource.data()), "Couldn't send texture data to GPU");
	vkName(graphics, gbext.resource[0], VK_OBJECT_TYPE_IMAGE, getName() + " staging buffer");

	//Allocate memory (TODO: by GraphicsExt)

	VkMemoryAllocateInfo memoryInfo;
	memset(&memoryInfo, 0, sizeof(memoryInfo));

	VkMemoryRequirements requirements;
	vkGetBufferMemoryRequirements(graphics.device, gbext.resource[0], &requirements);

	memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryInfo.allocationSize = requirements.size;

	uint32_t memoryIndex = u32_MAX;

	for (uint32_t i = 0; i < graphics.pmemory.memoryTypeCount; ++i)
		if ((requirements.memoryTypeBits & (1 << i)) && (graphics.pmemory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) {
			memoryIndex = i;
			break;
		}

	if (memoryIndex == u32_MAX)
		Log::throwError<VkTexture, 0x3>(String("Couldn't find a valid memory type for a texture staging buffer: ") + getName());

	memoryInfo.memoryTypeIndex = memoryIndex;

	vkCheck<0x9, VkTexture>(vkAllocateMemory(graphics.device, &memoryInfo, vkAllocator, &gbext.memory), "Couldn't allocate memory");
	vkCheck<0xA, VkTexture>(vkBindBufferMemory(graphics.device, gbext.resource[0], gbext.memory, 0), String("Couldn't bind memory to staging buffer ") + getName());

	//Copy data to staging buffer

	void *stagingData;

	vkCheck<0xB, VkTexture>(vkMapMemory(graphics.device, gbext.memory, 0, size, 0, &stagingData), "Couldn't map texture staging buffer");
	memcpy(stagingData, dat.addr(), size);
	vkUnmapMemory(graphics.device, gbext.memory);

	//Copy data to cmd list

	CommandListExt &cmd = graphics.stagingCmdList->getExtension();

	VkFilter filter = info.mipFilter != TextureMipFilter::Nearest ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;

	//Transition to write

	VkImageMemoryBarrier barrier;
	memset(&barrier, 0, sizeof(barrier));

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.image = ext.resource;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.levelCount = info.mipLevels;
	barrier.subresourceRange.layerCount = 1;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	vkCmdPipelineBarrier(cmd.cmds[0], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	//Copy it into the texture

	VkBufferImageCopy region;
	memset(&region, 0, sizeof(region));

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.layerCount = 1U;
	region.imageOffset = { (i32) info.changedStart.x, (i32) info.changedStart.y, 0 };
	region.imageExtent = { changedLength.x, changedLength.y, 1 };

	vkCmdCopyBufferToImage(cmd.cmds[0], gbext.resource[0], ext.resource, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	//Generate mipmaps

	memset(&barrier, 0, sizeof(barrier));

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = ext.resource;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	u32 mipWidth = info.res.x, mipHeight = info.res.y;

	for (u32 i = 1; i < info.mipLevels; ++i) {

		//Transition mipmap source from DST_OPTIMAL to SRC_OPTIMAL

		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(cmd.cmds[0], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		//Create mipmap from mip i - 1 into i

		VkImageBlit blit;
		memset(&blit, 0, sizeof(blit));

		blit.srcOffsets[1] = { (i32)mipWidth, (i32)mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1U;
		blit.srcSubresource.layerCount = 1U;

		blit.dstOffsets[1] = { (i32)mipWidth >> 1, (i32)mipHeight >> 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.layerCount = 1U;

		vkCmdBlitImage(cmd.cmds[0], ext.resource, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, ext.resource, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, filter);

		if (mipWidth > 1) mipWidth >>= 1U;
		if (mipHeight > 1) mipHeight >>= 1U;

	}

	//Last mip is in DST_OPTIMAL and ones before that in SRC_OPTIMAL
	//So transition them back to SHADER_READ_OPTIMAL
	//Two barriers; 0 to mipLevels - 1 from SRC to SHADER_READ and mipLevels - 1 from DST to SHADER_READ

	memset(&barrier, 0, sizeof(barrier));

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = ext.resource;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.levelCount = info.mipLevels - 1U;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	VkImageMemoryBarrier barrier0 = barrier;
	barrier0.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier0.subresourceRange.levelCount = 1U;
	barrier0.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier0.subresourceRange.baseMipLevel = info.mipLevels - 1U;

	VkImageMemoryBarrier barriers[] = { barrier, barrier0 };

	vkCmdPipelineBarrier(cmd.cmds[0], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, info.mipLevels == 1 ? 1U : 2U, info.mipLevels == 1 ? barriers + 1 : barriers);

	//Clean up staging buffer and free memory

	graphics.stagingBuffers[graphics.current].push_back(gbext);

	if (!fullTexture)
		dat.deconstruct();

	info.changedStart = Vec2u(u32_MAX, u32_MAX);
	info.changedEnd = Vec2u();

}

#endif