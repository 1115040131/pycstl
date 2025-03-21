#include "sdl3/common/core/object_affiliate.h"
#include "sdl3/common/core/texture.h"

namespace pyc {
namespace sdl3 {

class Sprite : public ObjectAffiliate {
public:
    static std::shared_ptr<Sprite> Create(ObjectScreen* parent, const std::string& file_path, float scale = 1.F);

    virtual void render() override;

    const Texture& getTexture() const { return texture_; }
    virtual void setTexture(const Texture& texture);

    void setAngle(float angle) { texture_.angle = angle; }
    void setFlip(bool flip) { texture_.is_flip = flip; }

protected:
    Texture texture_;
};

}  // namespace sdl3
}  // namespace pyc
