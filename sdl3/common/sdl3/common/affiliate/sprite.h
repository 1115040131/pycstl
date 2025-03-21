#include "sdl3/common/core/object_affiliate.h"
#include "sdl3/common/core/texture.h"

namespace pyc {
namespace sdl3 {


class Sprite : public ObjectAffiliate {
public:
    virtual void render() override;

    const Texture& getTexture() const { return texture_; }
    void setTexture(const Texture& texture);

protected:
    Texture texture_;
};

}  // namespace sdl3
}  // namespace pyc
