#include <vkhr/embree/hair_style.hh>

#include <vkhr/ray_tracer.hh>

namespace vkhr {
    namespace embree {
        HairStyle::HairStyle(const vkhr::HairStyle& hair_style,
                             const vkhr::Raytracer& raytracer) {
            load(hair_style, raytracer);
        }

        void HairStyle::load(const vkhr::HairStyle& hair_style,
                             const vkhr::Raytracer& raytracer) {
            const auto& indices = hair_style.get_indices();

            position_thickness = hair_style.create_position_thickness_data();

            auto hair_geometry = rtcNewGeometry(raytracer.device, RTC_GEOMETRY_TYPE_FLAT_LINEAR_CURVE);

            rtcSetSharedGeometryBuffer(hair_geometry, RTC_BUFFER_TYPE_VERTEX, 0,
                                       RTC_FORMAT_FLOAT4, position_thickness.data(),
                                       0, sizeof(position_thickness[0]),
                                       position_thickness.size());

            rtcSetSharedGeometryBuffer(hair_geometry, RTC_BUFFER_TYPE_INDEX, 0,
                                       RTC_FORMAT_UINT, indices.data(),
                                       0, sizeof(indices[0]) * 2,
                                       indices.size() / 2);

            rtcCommitGeometry(hair_geometry);
            geometry = rtcAttachGeometry(raytracer.scene, hair_geometry);
            rtcReleaseGeometry(hair_geometry);
        }

        glm::vec3 HairStyle::shade(const Ray& surface_intersection,
                                   const LightSource& light_source,
                                   const Camera& projection_camera)  {
            auto hair_diffuse = glm::vec3(.32, .228, .128); // TressFX
            auto tangent = surface_intersection.get_uniform_tangent();
            tangent = projection_camera.get_view_matrix() * tangent;
            auto light_direction = projection_camera.get_view_matrix() * light_source.get_vector();
            auto shading = kajiya_kay(hair_diffuse,
                                      light_source.get_intensity(),
                                      50.0f, glm::normalize(tangent),
                                      glm::vec3 { light_direction },
                                      glm::vec3 { 0.0, 0.0, -1.0f });
            return shading;
        }

        unsigned HairStyle::get_geometry() const {
            return geometry;
        }

        glm::vec3 HairStyle::kajiya_kay(const glm::vec3& diffuse,
                                        const glm::vec3& specular,
                                        float p,
                                        const glm::vec3& tangent,
                                        const glm::vec3& light,
                                        const glm::vec3& eye) {
            float cosTL = glm::dot(light, tangent);
            float cosTE = glm::dot(eye,   tangent);

            float cosTL_squared = cosTL*cosTL;
            float cosTE_squared = cosTE*cosTE;

            float one_minus_cosTL_squared = 1.0f - cosTL_squared;
            float one_minus_cosTE_squared = 1.0f - cosTE_squared;

            float sinTL = std::sqrt(one_minus_cosTL_squared);
            float sinTE = std::sqrt(one_minus_cosTE_squared);

            glm::vec3 diffuse_colors  = diffuse  * sinTL;
            glm::vec3 specular_colors = specular * glm::clamp(std::pow((cosTL * cosTE + sinTL * sinTE), p), 0.0f, 1.0f);

            return diffuse_colors + specular_colors;
        }
    }
}