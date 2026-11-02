#include "booster_pack.h"
#include "card.h"
#include "consumable.h"
#include "game/common_ui.h"
#include "joker.h"
#include "list.h"
#include "skip_tag.h"
#include "sprite.h"
#include "voucher.h"

POOL_ENTRY(Sprite, MAX_SPRITES, IWRAM);
POOL_ENTRY(Joker, MAX_ACTIVE_JOKERS, EWRAM);
POOL_ENTRY(JokerObject, MAX_ACTIVE_JOKERS, EWRAM);
POOL_ENTRY(Card, MAX_CARDS, EWRAM);
POOL_ENTRY(CardObject, MAX_CARDS_ON_SCREEN, IWRAM);
POOL_ENTRY(SkipTag, MAX_SKIP_TAGS, EWRAM);
POOL_ENTRY(ConsumableObject, MAX_ACTIVE_CONSUMABLES, EWRAM);
POOL_ENTRY(BoosterPackObject, MAX_SHOP_PACKS, EWRAM);
POOL_ENTRY(VoucherObject, MAX_ACTIVE_VOUCHERS, EWRAM);
POOL_ENTRY(ListNode, MAX_LIST_NODES, EWRAM);
