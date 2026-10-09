#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <matjson.hpp>
using namespace geode::prelude;

// Crea un sprite animado con frames name_001.png ... name_NNN.png (los resuelve Texture Loader)
static CCSprite* makeAnim(std::string const& name, int frames, float fps, bool loop) {
    auto anim = CCAnimation::create();
    anim->setDelayPerUnit(1.f / fps);
    for (int i = 1; i <= frames; i++) {
        auto tex = CCTextureCache::sharedTextureCache()->addImage(fmt::format("{}_{:03}.png", name, i).c_str(), false);
        if (!tex) return nullptr;
        auto sz = tex->getContentSize();
        anim->addSpriteFrame(CCSpriteFrame::createWithTexture(tex, {0, 0, sz.width, sz.height}));
    }
    auto spr = CCSprite::createWithSpriteFrame(anim->getFrames()->count() ?
        static_cast<CCAnimationFrame*>(anim->getFrames()->objectAtIndex(0))->getSpriteFrame() : nullptr);
    if (!spr) return nullptr;
    auto act = CCAnimate::create(anim);
    spr->runAction(loop ? static_cast<CCAction*>(CCRepeatForever::create(act)) : act);
    return spr;
}

static void addAnims(CCNode* parent, std::string const& target) {
    auto path = Mod::get()->getResourcesDir() / "animations.json";
    auto res = file::readJson(path);
    if (!res) return;
    auto root = res.unwrap();
    auto win = CCDirector::get()->getWinSize();
    for (auto& [name, cfg] : root) {
        if (cfg["target"].asString().unwrapOr("") != target) continue;
        auto spr = makeAnim(name, cfg["frames"].asInt().unwrapOr(1), (float)cfg["fps"].asDouble().unwrapOr(24), cfg["loop"].asBool().unwrapOr(true));
        if (!spr) { log::warn("Faltan frames de {}", name); continue; }
        float px = 0.5f, py = 0.5f;
        if (cfg.contains("pos")) { px = (float)cfg["pos"][0].asDouble().unwrapOr(.5); py = (float)cfg["pos"][1].asDouble().unwrapOr(.5); }
        spr->setPosition({win.width * px, win.height * py});
        spr->setZOrder((int)cfg["z"].asInt().unwrapOr(5));
        parent->addChild(spr);
    }
}

class $modify(MikuMenu, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        addAnims(this, "MenuLayer");
        return true;
    }
};

class $modify(MikuPlay, PlayLayer) {
    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        addAnims(this->m_uiLayer, "PlayLayer");
    }
};
