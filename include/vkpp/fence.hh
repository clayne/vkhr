#ifndef VKPP_FENCE_HH
#define VKPP_FENCE_HH

#include <vulkan/vulkan.h>

#include <cstdint>

namespace vkpp {
    class Device;
    class Fence final {
    public:
        Fence() = default;
        Fence(Device& device);

        ~Fence() noexcept;

        Fence(Fence&& fence) noexcept;
        Fence& operator=(Fence&& fence) noexcept;

        friend void swap(Fence& lhs, Fence& rhs);

        VkFence& get_handle();

        bool await(std::uint64_t);

        bool get_status() const;

        void reset();

    private:
        VkDevice device { VK_NULL_HANDLE };
        VkFence  handle { VK_NULL_HANDLE };
    };
}

#endif