#include "game.h"
#include "game/round.h"
#include "game_variables.h"
#include "hand.h"
#include "hand_levels.h"
#include "joker.h"
#include "joker_cards.h"
#include "list.h"
#include "pool.h"
#include "random.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MISPRINT_MAX_MULT 23

#define DECLARE_JOKER_DESC_FUNC(joker_desc_name) \
    static int joker_desc_name(Joker* joker, Rect dest_rect);

#define DECLARE_JOKER_EFFECT_FUNC(joker_effect_name) \
    static u32 joker_effect_name(                    \
        Joker* joker,                                \
        Card* scored_card,                           \
        enum JokerEvent joker_event,                 \
        JokerEffect** joker_effect                   \
    );

#define SCORE_ON_EVENT_ONLY_WITH_CARD(scored_card, restricted_event, checked_event) \
    if (checked_event != restricted_event || scored_card == NULL)                   \
    {                                                                               \
        return JOKER_EFFECT_FLAG_NONE;                                              \
    }
#define SCORE_ON_EVENT_ONLY(restricted_event, checked_event) \
    if (checked_event != restricted_event)                   \
    {                                                        \
        return JOKER_EFFECT_FLAG_NONE;                       \
    }

static JokerEffect s_shared_joker_effect = {0};

// Joker Descriptions

DECLARE_JOKER_DESC_FUNC(default_joker_desc)
DECLARE_JOKER_DESC_FUNC(greedy_joker_desc)
DECLARE_JOKER_DESC_FUNC(lusty_joker_desc)
DECLARE_JOKER_DESC_FUNC(wrathful_joker_desc)
DECLARE_JOKER_DESC_FUNC(gluttonous_joker_desc)
DECLARE_JOKER_DESC_FUNC(jolly_joker_desc)
DECLARE_JOKER_DESC_FUNC(zany_joker_desc)
DECLARE_JOKER_DESC_FUNC(mad_joker_desc)
DECLARE_JOKER_DESC_FUNC(crazy_joker_desc)
DECLARE_JOKER_DESC_FUNC(droll_joker_desc)
DECLARE_JOKER_DESC_FUNC(sly_joker_desc)
DECLARE_JOKER_DESC_FUNC(wily_joker_desc)
DECLARE_JOKER_DESC_FUNC(clever_joker_desc)
DECLARE_JOKER_DESC_FUNC(devious_joker_desc)
DECLARE_JOKER_DESC_FUNC(crafty_joker_desc)
DECLARE_JOKER_DESC_FUNC(half_joker_desc)
DECLARE_JOKER_DESC_FUNC(stencil_joker_desc)
DECLARE_JOKER_DESC_FUNC(misprint_joker_desc)
DECLARE_JOKER_DESC_FUNC(walkie_talkie_joker_desc)
DECLARE_JOKER_DESC_FUNC(fibonnaci_joker_desc)
DECLARE_JOKER_DESC_FUNC(banner_joker_desc)
DECLARE_JOKER_DESC_FUNC(mystic_summit_joker_desc)
DECLARE_JOKER_DESC_FUNC(blackboard_joker_desc)
DECLARE_JOKER_DESC_FUNC(blue_joker_desc)
DECLARE_JOKER_DESC_FUNC(raised_fist_joker_desc)
DECLARE_JOKER_DESC_FUNC(reserved_parking_joker_desc)
DECLARE_JOKER_DESC_FUNC(business_card_joker_desc)
DECLARE_JOKER_DESC_FUNC(scholar_joker_desc)
DECLARE_JOKER_DESC_FUNC(scary_face_joker_desc)
DECLARE_JOKER_DESC_FUNC(abstract_joker_desc)
DECLARE_JOKER_DESC_FUNC(bull_joker_desc)
DECLARE_JOKER_DESC_FUNC(smiley_face_joker_desc)
DECLARE_JOKER_DESC_FUNC(even_steven_joker_desc)
DECLARE_JOKER_DESC_FUNC(odd_todd_joker_desc)
DECLARE_JOKER_DESC_FUNC(acrobat_joker_desc)
DECLARE_JOKER_DESC_FUNC(hanging_chad_joker_desc)
DECLARE_JOKER_DESC_FUNC(the_duo_joker_desc)
DECLARE_JOKER_DESC_FUNC(the_trio_joker_desc)
DECLARE_JOKER_DESC_FUNC(the_family_joker_desc)
DECLARE_JOKER_DESC_FUNC(the_order_joker_desc)
DECLARE_JOKER_DESC_FUNC(the_tribe_joker_desc)
DECLARE_JOKER_DESC_FUNC(bootstraps_joker_desc)
DECLARE_JOKER_DESC_FUNC(shoot_the_moon_joker_desc)
DECLARE_JOKER_DESC_FUNC(pareidolia_joker_desc)
DECLARE_JOKER_DESC_FUNC(photograph_joker_desc)
DECLARE_JOKER_DESC_FUNC(dusk_joker_desc)
DECLARE_JOKER_DESC_FUNC(shortcut_joker_desc)
DECLARE_JOKER_DESC_FUNC(blueprint_joker_desc)
DECLARE_JOKER_DESC_FUNC(brainstorm_joker_desc)
DECLARE_JOKER_DESC_FUNC(hack_joker_desc)
DECLARE_JOKER_DESC_FUNC(four_fingers_joker_desc)
DECLARE_JOKER_DESC_FUNC(seltzer_joker_desc)
DECLARE_JOKER_DESC_FUNC(sock_and_buskin_joker_desc)
DECLARE_JOKER_DESC_FUNC(baron_joker_desc)
DECLARE_JOKER_DESC_FUNC(ride_the_bus_joker_desc)
DECLARE_JOKER_DESC_FUNC(supernova_joker_desc)
DECLARE_JOKER_DESC_FUNC(runner_joker_desc)
DECLARE_JOKER_DESC_FUNC(square_joker_desc)
DECLARE_JOKER_DESC_FUNC(ice_cream_joker_desc)
DECLARE_JOKER_DESC_FUNC(loyalty_card_joker_desc)
DECLARE_JOKER_DESC_FUNC(spare_trousers_joker_desc)
DECLARE_JOKER_DESC_FUNC(wee_joker_desc)
DECLARE_JOKER_DESC_FUNC(onyx_agate_joker_desc)
DECLARE_JOKER_DESC_FUNC(arrowhead_joker_desc)
DECLARE_JOKER_DESC_FUNC(golden_joker_desc)
DECLARE_JOKER_DESC_FUNC(delayed_gratification_joker_desc)
DECLARE_JOKER_DESC_FUNC(to_the_moon_joker_desc)
DECLARE_JOKER_DESC_FUNC(rocket_joker_desc)
DECLARE_JOKER_DESC_FUNC(egg_joker_desc)
DECLARE_JOKER_DESC_FUNC(credit_card_joker_desc)
DECLARE_JOKER_DESC_FUNC(juggler_joker_desc)
DECLARE_JOKER_DESC_FUNC(drunkard_joker_desc)
DECLARE_JOKER_DESC_FUNC(troubadour_joker_desc)
DECLARE_JOKER_DESC_FUNC(merry_andy_joker_desc)
DECLARE_JOKER_DESC_FUNC(stuntman_joker_desc)
DECLARE_JOKER_DESC_FUNC(gift_card_joker_desc)
DECLARE_JOKER_DESC_FUNC(swashbuckler_joker_desc)
DECLARE_JOKER_DESC_FUNC(golden_ticket_joker_desc)
DECLARE_JOKER_DESC_FUNC(rough_gem_joker_desc)
DECLARE_JOKER_DESC_FUNC(steel_joker_desc)
DECLARE_JOKER_DESC_FUNC(glass_joker_desc)
DECLARE_JOKER_DESC_FUNC(lucky_cat_joker_desc)
DECLARE_JOKER_DESC_FUNC(vampire_joker_desc)
DECLARE_JOKER_DESC_FUNC(midas_mask_joker_desc)
DECLARE_JOKER_DESC_FUNC(cartomancer_joker_desc)
DECLARE_JOKER_DESC_FUNC(constellation_joker_desc)
DECLARE_JOKER_DESC_FUNC(fortune_teller_joker_desc)
DECLARE_JOKER_DESC_FUNC(hallucination_joker_desc)
DECLARE_JOKER_DESC_FUNC(stone_joker_desc)
DECLARE_JOKER_DESC_FUNC(marble_joker_desc)
DECLARE_JOKER_DESC_FUNC(certificate_joker_desc)
DECLARE_JOKER_DESC_FUNC(space_joker_desc)
DECLARE_JOKER_DESC_FUNC(hiker_joker_desc)
DECLARE_JOKER_DESC_FUNC(burglar_joker_desc)

// Joker Effect functions

static u32 sinful_joker_effect(
    Card* scored_card,
    u8 sinful_suit,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
);

DECLARE_JOKER_EFFECT_FUNC(joker_effect_noop)
DECLARE_JOKER_EFFECT_FUNC(default_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(greedy_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(lusty_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(wrathful_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(gluttonous_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(jolly_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(zany_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(mad_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(crazy_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(droll_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(sly_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(wily_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(clever_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(devious_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(crafty_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(half_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(stencil_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(misprint_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(walkie_talkie_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(fibonnaci_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(banner_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(mystic_summit_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(blackboard_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(blue_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(raised_fist_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(reserved_parking_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(business_card_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(scholar_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(scary_face_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(abstract_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(bull_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(smiley_face_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(even_steven_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(odd_todd_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(acrobat_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(hanging_chad_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(the_duo_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(the_trio_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(the_family_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(the_order_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(the_tribe_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(bootstraps_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(shoot_the_moon_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(photograph_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(dusk_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(blueprint_brainstorm_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(hack_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(seltzer_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(sock_and_buskin_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(baron_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(ride_the_bus_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(supernova_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(runner_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(square_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(ice_cream_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(loyalty_card_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(spare_trousers_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(wee_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(onyx_agate_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(arrowhead_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(rocket_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(stuntman_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(swashbuckler_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(golden_ticket_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(rough_gem_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(steel_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(glass_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(lucky_cat_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(vampire_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(midas_mask_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(constellation_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(fortune_teller_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(stone_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(space_joker_effect)
DECLARE_JOKER_EFFECT_FUNC(hiker_joker_effect)

/* The index of a joker in the registry matches its ID.
 *
 * The joker sprites are matched by ID so the position in the registry
 * determines the joker's sprite.
 *
 * Each consecutive NUM_JOKERS_PER_SPRITESHEET (defined in joker.c) jokers
 * share a spritesheet and thus a color palette.
 *
 * To make better use of color palettes jokers may be rearranged here
 * (and put together in the matching spritesheet) to share a color palette.
 * Otherwise the order is similar to the wiki.
 *
 * TODO: move Name and Description printing out of this when the CardInstance is implemented.
 */
// clang-format off
const JokerInfo joker_registry[] = 
{
    // Spritesheet 0
    { "Джокер",            COMMON_JOKER,    2, false, default_joker_desc,          default_joker_effect              }, // DEFAULT_JOKER_ID = 0
    { "Абстрактный",   COMMON_JOKER,    4, false, abstract_joker_desc,         abstract_joker_effect             }, // 1
    { "Полу-Джокер",       COMMON_JOKER,    5, false, half_joker_desc,             half_joker_effect                 }, // 2
    { "Опечатка",         COMMON_JOKER,    4, true,  misprint_joker_desc,         misprint_joker_effect             }, // 3
    { "Страшное лицо",       COMMON_JOKER,    4, false, scary_face_joker_desc,       scary_face_joker_effect           }, // 4
    { "Комедия и драма",  UNCOMMON_JOKER,  6, false, sock_and_buskin_joker_desc,  sock_and_buskin_joker_effect      }, // 5
    { "Акробат",          UNCOMMON_JOKER,  6, false, acrobat_joker_desc,          acrobat_joker_effect              }, // 6
    { "Фибоначчи",        UNCOMMON_JOKER,  8, false, fibonnaci_joker_desc,        fibonnaci_joker_effect            }, // 7
    { "Учёный",          COMMON_JOKER,    4, false, scholar_joker_desc,          scholar_joker_effect              }, // 8
    { "Ловкий Джокер",     COMMON_JOKER,    4, false, crafty_joker_desc,           crafty_joker_effect               }, // 9
    { "Забавный Джокер",      COMMON_JOKER,    4, false, droll_joker_desc,            droll_joker_effect                }, // 10
    { "Поднятый кулак",      COMMON_JOKER,    5, false, raised_fist_joker_desc,      raised_fist_joker_effect          }, // 11
    { "Парковка", COMMON_JOKER,    6, false, reserved_parking_joker_desc, reserved_parking_joker_effect     }, // 12
    { "Визитка",    COMMON_JOKER,    4, false, business_card_joker_desc,    business_card_joker_effect        }, // 13
    { "Висячий Чед",     COMMON_JOKER,    4, false, hanging_chad_joker_desc,     hanging_chad_joker_effect         }, // 14
    { "Трафарет",    UNCOMMON_JOKER,  8, false, stencil_joker_desc,          stencil_joker_effect              }, // 15
    { "Знамя",           COMMON_JOKER,    5, false, banner_joker_desc,           banner_joker_effect               }, // 16
    { "Достать луну",   COMMON_JOKER,    5, false, shoot_the_moon_joker_desc,   shoot_the_moon_joker_effect,      }, // 17
    // Spritesheet 1 
    { "Жадный Джокер",     COMMON_JOKER,    5, false, greedy_joker_desc,           greedy_joker_effect               }, // 18
    { "Похотливый Джокер",      COMMON_JOKER,    5, false, lusty_joker_desc,            lusty_joker_effect                }, // 19
    // Spritesheet 2
    { "Гневный Джокер",   COMMON_JOKER,    5, false, wrathful_joker_desc,         wrathful_joker_effect             }, // 20
    { "Прожорливый Джокер", COMMON_JOKER,    5, false, gluttonous_joker_desc,       gluttonous_joker_effect           }, // 21
    // Spritesheet 3
    { "Безумный Джокер",      COMMON_JOKER,    4, false, crazy_joker_desc,            crazy_joker_effect                }, // 22
    { "Бешеный Джокер",        COMMON_JOKER,    4, false, mad_joker_desc,              mad_joker_effect                  }, // 23
    { "Умный Джокер",     COMMON_JOKER,    4, false, clever_joker_desc,           clever_joker_effect               }, // 24
    { "Коварный Джокер",    COMMON_JOKER,    4, false, devious_joker_desc,          devious_joker_effect              }, // 25
    { "Чётный Стивен",      COMMON_JOKER,    4, false, even_steven_joker_desc,      even_steven_joker_effect          }, // 26
    // Spritesheet 4
    { "Доска",       UNCOMMON_JOKER,  6, false, blackboard_joker_desc,       blackboard_joker_effect           }, // 27
    { "Мистический пик",    COMMON_JOKER,    5, false, mystic_summit_joker_desc,    mystic_summit_joker_effect        }, // 28
    { "Рация",    COMMON_JOKER,    4, false, walkie_talkie_joker_desc,    walkie_talkie_joker_effect        }, // 29
    { "Чудной Джокер",       COMMON_JOKER,    4, false, zany_joker_desc,             zany_joker_effect                 }, // 30
    { "Лукавый Джокер",       COMMON_JOKER,    4, false, wily_joker_desc,             wily_joker_effect                 }, // 31
    // Spritesheet 5
    { "Хитрый Джокер",        COMMON_JOKER,    3, false, sly_joker_desc,              sly_joker_effect                  }, // 32
    { "Весёлый Джокер",      COMMON_JOKER,    3, false, jolly_joker_desc,            jolly_joker_effect                }, // 33
    { "Синий Джокер",       COMMON_JOKER,    5, false, blue_joker_desc,             blue_joker_effect                 }, // 34
    { "Нечётный Тодд",         COMMON_JOKER,    4, false, odd_todd_joker_desc,         odd_todd_joker_effect             }, // 35
    // Spritesheet 6
    { "Дуэт",          RARE_JOKER,      8, false, the_duo_joker_desc,          the_duo_joker_effect              }, // 36
    { "Трио",         RARE_JOKER,      8, false, the_trio_joker_desc,         the_trio_joker_effect             }, // 37
    { "Порядок",        RARE_JOKER,      8, false, the_order_joker_desc,        the_order_joker_effect            }, // 38
    { "Племя",        RARE_JOKER,      8, false, the_tribe_joker_desc,        the_tribe_joker_effect            }, // 39
    // Spritesheet 7
    { "Семья",       RARE_JOKER,      8, false, the_family_joker_desc,       the_family_joker_effect           }, // 40
    { "Мозговой штурм",       RARE_JOKER,     10, false, brainstorm_joker_desc,       blueprint_brainstorm_joker_effect }, // 41 Brainstorm
    // Spritesheet 8
    { "Смайлик",      COMMON_JOKER,    4, false, smiley_face_joker_desc,      smiley_face_joker_effect          }, // 42
    { "Бык",             UNCOMMON_JOKER,  6, false, bull_joker_desc,             bull_joker_effect                 }, // 43
    // Individual Jokers (for now :3)
    { "Фотография",       COMMON_JOKER,    5, false, photograph_joker_desc,       photograph_joker_effect,          }, // 44
    { "Халтура",             UNCOMMON_JOKER,  6, false, hack_joker_desc,             hack_joker_effect                 }, // 45
    { "Парейдолия",       UNCOMMON_JOKER,  5, false, pareidolia_joker_desc,       joker_effect_noop                 }, // 46 Pareidolia
    { "Самоподъём",       UNCOMMON_JOKER,  7, false, bootstraps_joker_desc,       bootstraps_joker_effect           }, // 47
    { "Короткий путь",         UNCOMMON_JOKER,  7, false, shortcut_joker_desc,         joker_effect_noop,                }, // 48 Shortcut
    { "Сумерки",             UNCOMMON_JOKER,  5, false, dusk_joker_desc,             dusk_joker_effect                 }, // 49
    { "Четыре пальца",     UNCOMMON_JOKER,  7, false, four_fingers_joker_desc,     joker_effect_noop,                }, // 50 Four Fingers
    { "Сельтерская",          UNCOMMON_JOKER,  6, false, seltzer_joker_desc,          seltzer_joker_effect,             }, // 51
    { "Чертёж",        RARE_JOKER,     10, false, blueprint_joker_desc,        blueprint_brainstorm_joker_effect }, // 52 Blueprint

    { "Барон", RARE_JOKER, 8, false, baron_joker_desc, baron_joker_effect }, // 53
    { "Автобус", COMMON_JOKER, 6, false, ride_the_bus_joker_desc, ride_the_bus_joker_effect }, // 54
    { "Сверхновая", COMMON_JOKER, 5, false, supernova_joker_desc, supernova_joker_effect }, // 55
    { "Бегун", COMMON_JOKER, 5, false, runner_joker_desc, runner_joker_effect }, // 56
    { "Квадрат", COMMON_JOKER, 4, false, square_joker_desc, square_joker_effect }, // 57
    { "Мороженое", COMMON_JOKER, 5, false, ice_cream_joker_desc, ice_cream_joker_effect }, // 58
    { "Карта лояльности", UNCOMMON_JOKER, 5, false, loyalty_card_joker_desc, loyalty_card_joker_effect }, // 59
    { "Запасные штаны", UNCOMMON_JOKER, 6, false, spare_trousers_joker_desc, spare_trousers_joker_effect }, // 60
    { "Малыш Джокер", RARE_JOKER, 8, false, wee_joker_desc, wee_joker_effect }, // 61
    { "Чёрный агат", UNCOMMON_JOKER, 7, false, onyx_agate_joker_desc, onyx_agate_joker_effect }, // 62
    { "Наконечник", UNCOMMON_JOKER, 7, false, arrowhead_joker_desc, arrowhead_joker_effect }, // 63

    { "Золотой Джокер", COMMON_JOKER, 6, false, golden_joker_desc, joker_effect_noop }, // 64
    { "Отсрочка", COMMON_JOKER, 4, false, delayed_gratification_joker_desc, joker_effect_noop }, // 65
    { "К Луне", UNCOMMON_JOKER, 5, false, to_the_moon_joker_desc, joker_effect_noop }, // 66
    { "Ракета", UNCOMMON_JOKER, 6, false, rocket_joker_desc, rocket_joker_effect }, // 67
    { "Яйцо", COMMON_JOKER, 4, false, egg_joker_desc, joker_effect_noop }, // 68
    { "Кредитка", COMMON_JOKER, 1, false, credit_card_joker_desc, joker_effect_noop }, // 69
    { "Жонглёр", COMMON_JOKER, 4, false, juggler_joker_desc, joker_effect_noop }, // 70
    { "Пьяница", COMMON_JOKER, 4, false, drunkard_joker_desc, joker_effect_noop }, // 71
    { "Трубадур", UNCOMMON_JOKER, 6, false, troubadour_joker_desc, joker_effect_noop }, // 72
    { "Весёлый Энди", UNCOMMON_JOKER, 7, false, merry_andy_joker_desc, joker_effect_noop }, // 73
    { "Каскадёр", RARE_JOKER, 7, false, stuntman_joker_desc, stuntman_joker_effect }, // 74
    { "Подарочная карта", UNCOMMON_JOKER, 6, false, gift_card_joker_desc, joker_effect_noop }, // 75
    { "Головорез", COMMON_JOKER, 4, false, swashbuckler_joker_desc, swashbuckler_joker_effect }, // 76
    { "Золотой билет", COMMON_JOKER, 5, false, golden_ticket_joker_desc, golden_ticket_joker_effect }, // 77
    { "Грубый алмаз", UNCOMMON_JOKER, 7, false, rough_gem_joker_desc, rough_gem_joker_effect }, // 78


    { "Стальной Джокер", UNCOMMON_JOKER, 7, false, steel_joker_desc, steel_joker_effect }, // 79
    { "Стеклянный Джокер", UNCOMMON_JOKER, 6, false, glass_joker_desc, glass_joker_effect }, // 80
    { "Счастливый кот", UNCOMMON_JOKER, 6, false, lucky_cat_joker_desc, lucky_cat_joker_effect }, // 81
    { "Вампир", UNCOMMON_JOKER, 7, false, vampire_joker_desc, vampire_joker_effect }, // 82
    { "Маска Мидаса", UNCOMMON_JOKER, 7, false, midas_mask_joker_desc, midas_mask_joker_effect }, // 83
    { "Картомант", UNCOMMON_JOKER, 6, false, cartomancer_joker_desc, joker_effect_noop }, // 84
    { "Созвездие", UNCOMMON_JOKER, 6, false, constellation_joker_desc, constellation_joker_effect }, // 85
    { "Гадалка", COMMON_JOKER, 6, false, fortune_teller_joker_desc, fortune_teller_joker_effect }, // 86
    { "Галлюцинация", COMMON_JOKER, 4, false, hallucination_joker_desc, joker_effect_noop }, // 87
    { "Каменный Джокер", UNCOMMON_JOKER, 6, false, stone_joker_desc, stone_joker_effect }, // 88
    { "Мраморный Джокер", UNCOMMON_JOKER, 6, false, marble_joker_desc, joker_effect_noop }, // 89
    { "Сертификат", UNCOMMON_JOKER, 6, false, certificate_joker_desc, joker_effect_noop }, // 90
    { "Космический Джокер", UNCOMMON_JOKER, 5, false, space_joker_desc, space_joker_effect }, // 91
    { "Турист", UNCOMMON_JOKER, 5, false, hiker_joker_desc, hiker_joker_effect }, // 92
    { "Взломщик", UNCOMMON_JOKER, 6, false, burglar_joker_desc, joker_effect_noop }, // 93

    // The following jokers don't have sprites yet,
    // uncomment them when their sprites are added.
#if 0
#endif
};
// clang-format on

static const size_t joker_registry_size = NUM_ELEM_IN_ARR(joker_registry);

const JokerInfo* get_joker_registry_entry(int joker_id)
{
    if (joker_id < 0 || (size_t)joker_id >= joker_registry_size)
    {
        return NULL;
    }
    return &joker_registry[joker_id];
}

size_t get_joker_registry_size(void)
{
    return joker_registry_size;
}

#pragma region JOKER DESCRIPTIONS

static int default_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] = TTE_RED_TAG "+4 " TTE_BLACK_TAG "множ.";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int greedy_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Сыгранные карты масти " TTE_YELLOW_TAG "Бубны " TTE_BLACK_TAG
                      "дают " TTE_RED_TAG "+3 " TTE_BLACK_TAG "множ. при подсчёте";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int lusty_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Сыгранные карты масти " TTE_RED_TAG "Червы " TTE_BLACK_TAG
                      "дают " TTE_RED_TAG "+3 " TTE_BLACK_TAG "множ. при подсчёте";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int wrathful_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Сыгранные карты масти " TTE_DARK_BLUE_TAG "Пики " TTE_BLACK_TAG
                      "дают " TTE_RED_TAG "+3 " TTE_BLACK_TAG "множ. при подсчёте";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int gluttonous_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Сыгранные карты масти " TTE_DARK_GREEN_TAG "Трефы " TTE_BLACK_TAG
                      "дают " TTE_RED_TAG "+3 " TTE_BLACK_TAG "множ. при подсчёте";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int jolly_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_RED_TAG "+8 " TTE_BLACK_TAG "множ., если в сыгранной руке есть " TTE_YELLOW_TAG "Пара";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int zany_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] = TTE_RED_TAG
        "+12 " TTE_BLACK_TAG "множ., если в сыгранной руке есть " TTE_YELLOW_TAG "Тройка";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int mad_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] = TTE_RED_TAG
        "+10 " TTE_BLACK_TAG "множ., если в сыгранной руке есть " TTE_YELLOW_TAG "Две пары";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int crazy_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] = TTE_RED_TAG
        "+12 " TTE_BLACK_TAG "множ., если в сыгранной руке есть " TTE_YELLOW_TAG "Стрит";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int droll_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_RED_TAG "+10 " TTE_BLACK_TAG "множ., если в сыгранной руке есть " TTE_YELLOW_TAG "Флеш";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int sly_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] = TTE_BLUE_TAG
        "+50 " TTE_BLACK_TAG "фишек, если в сыгранной руке есть " TTE_YELLOW_TAG "Пара";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int wily_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] = TTE_BLUE_TAG
        "+100 " TTE_BLACK_TAG "фишек, если в сыгранной руке есть " TTE_YELLOW_TAG "Тройка";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int clever_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] = TTE_BLUE_TAG
        "+80 " TTE_BLACK_TAG "фишек, если в сыгранной руке есть " TTE_YELLOW_TAG "Две пары";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int devious_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] = TTE_BLUE_TAG
        "+100 " TTE_BLACK_TAG "фишек, если в сыгранной руке есть " TTE_YELLOW_TAG "Стрит";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int crafty_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] = TTE_BLUE_TAG
        "+80 " TTE_BLACK_TAG "фишек, если в сыгранной руке есть " TTE_YELLOW_TAG "Флеш";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int half_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_RED_TAG "+20 " TTE_BLACK_TAG "множ., если в сыгранной руке " TTE_YELLOW_TAG
                    "3 " TTE_BLACK_TAG "карты или меньше";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int stencil_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] =
        TTE_RED_TAG "X1 " TTE_BLACK_TAG
                    "множ. за каждый пустой слот Джокера, включая Трафарет\n\n(Сейчас " TTE_RED_TAG
                    "X%ld " TTE_BLACK_TAG "множ.)";
    const u32 desc_max_size = 220;

    List* jokers = get_jokers_list();
    u32 stencil_bonus = g_game_vars.joker_slots - list_get_len(jokers);

    ListItr itr = list_itr_create(jokers);
    JokerObject* joker_object;
    while ((joker_object = list_itr_next(&itr)))
    {
        if (joker_object->joker->id == STENCIL_JOKER_ID)
            stencil_bonus++;
    }

    char desc[desc_max_size];
    snprintf(desc, desc_max_size, desc_format, stencil_bonus);

    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int misprint_joker_desc(Joker* joker, Rect dest_rect)
{
    // TODO: print glitchy desc with occasional next card reveal
    char desc[] = TTE_YELLOW_TAG "Случайный " TTE_BLACK_TAG "множитель от " TTE_RED_TAG
                                 "+0" TTE_BLACK_TAG " до " TTE_RED_TAG "+23";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int walkie_talkie_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Каждые сыгранные " TTE_YELLOW_TAG "10 " TTE_BLACK_TAG "или " TTE_YELLOW_TAG
                      "4 " TTE_BLACK_TAG "дают " TTE_BLUE_TAG "+10 " TTE_BLACK_TAG
                      "фишек и " TTE_RED_TAG "+4 " TTE_BLACK_TAG "множ. при подсчёте";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int fibonnaci_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Каждый сыгранный " TTE_YELLOW_TAG "Туз" TTE_BLACK_TAG ", " TTE_YELLOW_TAG
                      "2" TTE_BLACK_TAG ", " TTE_YELLOW_TAG "3" TTE_BLACK_TAG ", " TTE_YELLOW_TAG
                      "5" TTE_BLACK_TAG ", " TTE_YELLOW_TAG "8 " TTE_BLACK_TAG "даёт " TTE_RED_TAG
                      "+8 " TTE_BLACK_TAG "множ. при подсчёте";

    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int banner_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLUE_TAG "+30 " TTE_BLACK_TAG "фишек за каждый оставшийся " TTE_YELLOW_TAG "сброс";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int mystic_summit_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] = TTE_RED_TAG
        "+15 " TTE_BLACK_TAG "множ., когда осталось " TTE_YELLOW_TAG "0 " TTE_BLACK_TAG "сбросов";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int blackboard_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_RED_TAG "X3 " TTE_BLACK_TAG "множ., если все карты в руке - " TTE_DARK_BLUE_TAG
                    "пики " TTE_BLACK_TAG "или " TTE_DARK_GREEN_TAG "трефы";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int blue_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] = TTE_BLUE_TAG
        "+2 " TTE_BLACK_TAG "фишки за каждую карту, оставшуюся в " TTE_YELLOW_TAG
        "колоде" TTE_BLACK_TAG "\n\n(Сейчас " TTE_BLUE_TAG "+%ld" TTE_BLACK_TAG " фишек)";
    const u32 desc_max_size = 220;

    u32 blue_bonus = (get_deck_top() + 1) * 2;

    char desc[desc_max_size];
    snprintf(desc, desc_max_size, desc_format, blue_bonus);

    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int raised_fist_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Добавляет к множителю " TTE_YELLOW_TAG "удвоенный" TTE_BLACK_TAG
                      " ранг " TTE_YELLOW_TAG "младшей" TTE_BLACK_TAG " карты в руке";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int reserved_parking_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] = TTE_BLACK_TAG "Каждая " TTE_YELLOW_TAG "картинка" TTE_BLACK_TAG
                                             " в руке с шансом " TTE_GREEN_TAG
                                             "1 из 2" TTE_BLACK_TAG " даёт " TTE_YELLOW_TAG "$1";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int business_card_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] = TTE_BLACK_TAG
        "Сыгранные " TTE_YELLOW_TAG "картинки" TTE_BLACK_TAG " с шансом " TTE_GREEN_TAG
        "1 из 2" TTE_BLACK_TAG " дают " TTE_YELLOW_TAG "$2" TTE_BLACK_TAG " при подсчёте";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int scholar_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] = TTE_BLACK_TAG
        "Сыгранные " TTE_YELLOW_TAG "Тузы" TTE_BLACK_TAG " дают " TTE_BLUE_TAG "+20" TTE_BLACK_TAG
        " фишек и " TTE_RED_TAG "+4" TTE_BLACK_TAG " множ. при подсчёте";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int scary_face_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Сыгранные " TTE_YELLOW_TAG "картинки" TTE_BLACK_TAG " дают " TTE_BLUE_TAG
                      "+30" TTE_BLACK_TAG " фишек при подсчёте";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int abstract_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] =
        TTE_RED_TAG "+3" TTE_BLACK_TAG " множ. за каждого " TTE_YELLOW_TAG
                    "Джокера\n\n(Сейчас " TTE_RED_TAG "+%ld" TTE_BLACK_TAG " множ.)";
    const u32 desc_max_size = 220;

    u32 abstract_bonus = list_get_len(get_jokers_list()) * 3;

    char desc[desc_max_size];
    snprintf(desc, desc_max_size, desc_format, abstract_bonus);

    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int bull_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] =
        TTE_BLUE_TAG "+2" TTE_BLACK_TAG " фишки за каждый " TTE_YELLOW_TAG "$1" TTE_BLACK_TAG
                     " у вас\n\n(Сейчас " TTE_BLUE_TAG "+%ld" TTE_BLACK_TAG " фишек)";
    const u32 desc_max_size = 220;

    u32 bull_bonus = (g_game_vars.money > 0) ? g_game_vars.money * 2 : 0;

    char desc[desc_max_size];
    snprintf(desc, desc_max_size, desc_format, bull_bonus);

    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int smiley_face_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Сыгранные " TTE_YELLOW_TAG "картинки" TTE_BLACK_TAG " дают " TTE_RED_TAG
                      "+5" TTE_BLACK_TAG " множ. при подсчёте";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int even_steven_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Сыгранные карты " TTE_YELLOW_TAG "чётного ранга дают " TTE_RED_TAG
                      "+4" TTE_BLACK_TAG " множ. при подсчёте\n(10, 8, 6, 4, 2)";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int odd_todd_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Сыгранные карты " TTE_YELLOW_TAG "нечётного ранга дают " TTE_BLUE_TAG
                      "+31" TTE_BLACK_TAG " фишек при подсчёте\n(Т, 9, 7, 5, 3)";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int acrobat_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] = TTE_RED_TAG "X3" TTE_BLACK_TAG " множ. на " TTE_YELLOW_TAG
                                           "последней руке" TTE_BLACK_TAG " раунда";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int hanging_chad_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Повторно срабатывает " TTE_YELLOW_TAG "первая" TTE_BLACK_TAG
                      " сыгранная карта при подсчёте ещё " TTE_YELLOW_TAG "2" TTE_BLACK_TAG " раза";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int the_duo_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_RED_TAG "X2" TTE_BLACK_TAG " множ., если в сыгранной руке есть " TTE_YELLOW_TAG "Пара";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int the_trio_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] = TTE_RED_TAG
        "X3" TTE_BLACK_TAG " множ., если в сыгранной руке есть " TTE_YELLOW_TAG "Тройка";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int the_family_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_RED_TAG "X4" TTE_BLACK_TAG " множ., если в сыгранной руке есть " TTE_YELLOW_TAG "Каре";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int the_order_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_RED_TAG "X3" TTE_BLACK_TAG " множ., если в сыгранной руке есть " TTE_YELLOW_TAG "Стрит";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int the_tribe_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_RED_TAG "X2" TTE_BLACK_TAG " множ., если в сыгранной руке есть " TTE_YELLOW_TAG "Флеш";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int bootstraps_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] =
        TTE_RED_TAG "+2" TTE_BLACK_TAG " множ. за каждые " TTE_YELLOW_TAG "$5" TTE_BLACK_TAG
                    " у вас\n\n(Сейчас " TTE_RED_TAG "+%ld" TTE_BLACK_TAG " множ.)";
    const u32 desc_max_size = 220;

    u32 bootstrap_bonus = (g_game_vars.money > 0) ? (g_game_vars.money / 5) * 2 : 0;

    char desc[desc_max_size];
    snprintf(desc, desc_max_size, desc_format, bootstrap_bonus);

    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int shoot_the_moon_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Каждая " TTE_YELLOW_TAG "Дама" TTE_BLACK_TAG " в руке даёт " TTE_RED_TAG
                      "+13" TTE_BLACK_TAG " множ.";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int photograph_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Первая сыгранная " TTE_YELLOW_TAG "картинка" TTE_BLACK_TAG
                      " даёт " TTE_RED_TAG "X2" TTE_BLACK_TAG " множ. при подсчёте";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int dusk_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Повторно срабатывают все сыгранные карты на " TTE_YELLOW_TAG
                      "последней руке" TTE_BLACK_TAG " раунда";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int brainstorm_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Копирует способность самого левого " TTE_YELLOW_TAG "Джокера";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int blueprint_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Копирует способность " TTE_YELLOW_TAG "Джокера" TTE_BLACK_TAG " справа";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int hack_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Повторно срабатывает каждая сыгранная " TTE_YELLOW_TAG "2" TTE_BLACK_TAG
                      ", " TTE_YELLOW_TAG "3" TTE_BLACK_TAG ", " TTE_YELLOW_TAG "4" TTE_BLACK_TAG
                      " или " TTE_YELLOW_TAG "5";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int seltzer_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] = TTE_BLACK_TAG
        "Повторно срабатывают все сыгранные карты ещё " TTE_YELLOW_TAG "%ld" TTE_BLACK_TAG " рук";
    const u32 desc_max_size = 220;

    char desc[desc_max_size];
    snprintf(desc, desc_max_size, desc_format, joker->persistent_state);

    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int sock_and_buskin_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Повторно срабатывают все сыгранные " TTE_YELLOW_TAG "картинки";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int baron_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Каждый " TTE_YELLOW_TAG "Король" TTE_BLACK_TAG " в руке даёт " TTE_RED_TAG "X1.5" TTE_BLACK_TAG " множ.";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int ride_the_bus_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] =
        TTE_RED_TAG "+1" TTE_BLACK_TAG " множ. за каждую руку подряд без " TTE_YELLOW_TAG "картинок" TTE_BLACK_TAG "\n\n(Сейчас " TTE_RED_TAG "+%ld" TTE_BLACK_TAG " множ.)";
    const u32 desc_max_size = 220;

    char desc[desc_max_size];
    snprintf(desc, desc_max_size, desc_format, joker->persistent_state);

    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int supernova_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_RED_TAG "+1" TTE_BLACK_TAG " множ. за каждый раз, когда эта " TTE_YELLOW_TAG "комбинация" TTE_BLACK_TAG " сыграна за забег";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int runner_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] =
        TTE_BLACK_TAG "Получает " TTE_BLUE_TAG "+15" TTE_BLACK_TAG " фишек, если в руке есть " TTE_YELLOW_TAG "Стрит" TTE_BLACK_TAG "\n\n(Сейчас " TTE_BLUE_TAG "+%ld" TTE_BLACK_TAG " фишек)";
    const u32 desc_max_size = 220;

    char desc[desc_max_size];
    snprintf(desc, desc_max_size, desc_format, joker->persistent_state);

    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int square_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] =
        TTE_BLACK_TAG "Получает " TTE_BLUE_TAG "+4" TTE_BLACK_TAG " фишки, если сыграно ровно " TTE_YELLOW_TAG "4" TTE_BLACK_TAG " карты\n\n(Сейчас " TTE_BLUE_TAG "+%ld" TTE_BLACK_TAG " фишек)";
    const u32 desc_max_size = 220;

    char desc[desc_max_size];
    snprintf(desc, desc_max_size, desc_format, joker->persistent_state);

    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int ice_cream_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] =
        TTE_BLUE_TAG "+%ld" TTE_BLACK_TAG " фишек\n\n-" TTE_BLUE_TAG "5" TTE_BLACK_TAG " фишек после каждой сыгранной руки";
    const u32 desc_max_size = 220;

    char desc[desc_max_size];
    snprintf(desc, desc_max_size, desc_format, joker->persistent_state);

    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int loyalty_card_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] =
        TTE_RED_TAG "X4" TTE_BLACK_TAG " множ. каждую " TTE_YELLOW_TAG "6-ю" TTE_BLACK_TAG " руку\n\n(Ещё " TTE_YELLOW_TAG "%ld" TTE_BLACK_TAG " рук до бонуса)";
    const u32 desc_max_size = 220;

    char desc[desc_max_size];
    snprintf(desc, desc_max_size, desc_format, (joker->persistent_state >= 5) ? 0 : (5 - joker->persistent_state));

    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int spare_trousers_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] =
        TTE_BLACK_TAG "Получает " TTE_RED_TAG "+2" TTE_BLACK_TAG " множ., если в руке есть " TTE_YELLOW_TAG "Две пары" TTE_BLACK_TAG "\n\n(Сейчас " TTE_RED_TAG "+%ld" TTE_BLACK_TAG " множ.)";
    const u32 desc_max_size = 220;

    char desc[desc_max_size];
    snprintf(desc, desc_max_size, desc_format, joker->persistent_state);

    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int wee_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] =
        TTE_BLACK_TAG "Получает " TTE_BLUE_TAG "+8" TTE_BLACK_TAG " фишек за каждую сыгранную " TTE_YELLOW_TAG "2" TTE_BLACK_TAG "\n\n(Сейчас " TTE_BLUE_TAG "+%ld" TTE_BLACK_TAG " фишек)";
    const u32 desc_max_size = 220;

    char desc[desc_max_size];
    snprintf(desc, desc_max_size, desc_format, joker->persistent_state);

    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int onyx_agate_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Сыгранные карты масти " TTE_YELLOW_TAG "Трефы " TTE_BLACK_TAG "дают " TTE_RED_TAG "+7 " TTE_BLACK_TAG "множ. при подсчёте";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int arrowhead_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Сыгранные карты масти " TTE_YELLOW_TAG "Пики " TTE_BLACK_TAG "дают " TTE_BLUE_TAG "+50 " TTE_BLACK_TAG "фишек при подсчёте";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int swashbuckler_get_mult(const Joker* joker)
{
    int mult = 0;
    ListItr itr = list_itr_create(get_jokers_list());
    JokerObject* other;
    while ((other = list_itr_next(&itr)))
    {
        if (other->joker != joker)
            mult += joker_get_sell_value(other->joker);
    }
    return mult;
}

static int golden_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "В конце раунда даёт " TTE_YELLOW_TAG "$4";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int delayed_gratification_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_YELLOW_TAG "$2" TTE_BLACK_TAG " за каждый оставшийся сброс в конце раунда, если сбросы не " "использовались";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int to_the_moon_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Дополнительно " TTE_YELLOW_TAG "$1" TTE_BLACK_TAG " процента за каждые " TTE_YELLOW_TAG "$5" TTE_BLACK_TAG " в конце раунда";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int rocket_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] =
        TTE_BLACK_TAG "В конце раунда даёт " TTE_YELLOW_TAG "$%ld" TTE_BLACK_TAG "\n\nВыплата растёт на " TTE_YELLOW_TAG "$1" TTE_BLACK_TAG " за каждого босса";
    const u32 desc_max_size = 220;

    char desc[desc_max_size];
    snprintf(desc, desc_max_size, desc_format, joker->persistent_state);

    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int egg_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Цена продажи растёт на " TTE_YELLOW_TAG "$3" TTE_BLACK_TAG " в конце каждого раунда";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int credit_card_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Можно уйти в долг до " TTE_YELLOW_TAG "$20";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int juggler_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLUE_TAG "+1" TTE_BLACK_TAG " к размеру руки";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int drunkard_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_RED_TAG "+1" TTE_BLACK_TAG " сброс за раунд";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int troubadour_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLUE_TAG "+2" TTE_BLACK_TAG " к размеру руки, " TTE_RED_TAG "-1" TTE_BLACK_TAG " рука за раунд";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int merry_andy_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_RED_TAG "+3" TTE_BLACK_TAG " сброса за раунд, " TTE_BLUE_TAG "-1" TTE_BLACK_TAG " к размеру руки";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int stuntman_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLUE_TAG "+250" TTE_BLACK_TAG " фишек, " TTE_BLUE_TAG "-2" TTE_BLACK_TAG " к размеру руки";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int gift_card_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Цена продажи всех джокеров растёт на " TTE_YELLOW_TAG "$1" TTE_BLACK_TAG " в конце каждого раунда";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int swashbuckler_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] =
        TTE_BLACK_TAG "Множитель равен сумме цен продажи остальных джокеров\n\n(Сейчас " TTE_RED_TAG "+%d" TTE_BLACK_TAG " множ.)";
    const u32 desc_max_size = 220;

    char desc[desc_max_size];
    snprintf(desc, desc_max_size, desc_format, swashbuckler_get_mult(joker));

    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int golden_ticket_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Сыгранные " TTE_YELLOW_TAG "Золотые" TTE_BLACK_TAG " карты дают " TTE_YELLOW_TAG "$4" TTE_BLACK_TAG " при подсчёте";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int rough_gem_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Сыгранные карты масти " TTE_YELLOW_TAG "Бубны " TTE_BLACK_TAG "дают " TTE_YELLOW_TAG "$1" TTE_BLACK_TAG " при подсчёте";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int steel_joker_steel_count(void)
{
    return game_count_deck_enhancement(CARD_ENHANCEMENT_STEEL);
}

static int stone_joker_stone_count(void)
{
    return game_count_deck_enhancement(CARD_ENHANCEMENT_STONE);
}

// Prints "X1.25" style numbers: percent / 100 with two decimals
#define XMULT_FMT "X%d.%02d"
#define XMULT_ARGS(percent) (percent) / 100, (percent) % 100

static int print_desc(const char* desc, Rect dest_rect)
{
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int steel_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] =
        TTE_RED_TAG "X0.2" TTE_BLACK_TAG " множ. за каждую " TTE_YELLOW_TAG "Стальную" TTE_BLACK_TAG " карту в колоде\n\n(Сейчас " TTE_RED_TAG XMULT_FMT TTE_BLACK_TAG " множ.)";
    char desc[180];
    int percent = joker_cards_xmult_percent(joker->id, 0, steel_joker_steel_count());
    snprintf(desc, sizeof(desc), desc_format, XMULT_ARGS(percent));
    return print_desc(desc, dest_rect);
}

static int glass_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] =
        TTE_RED_TAG "X0.75" TTE_BLACK_TAG " множ. за каждую разбитую " TTE_YELLOW_TAG "Стеклянную" TTE_BLACK_TAG " карту\n\n(Сейчас " TTE_RED_TAG XMULT_FMT TTE_BLACK_TAG " множ.)";
    char desc[180];
    int percent = joker_cards_xmult_percent(joker->id, joker->persistent_state, 0);
    snprintf(desc, sizeof(desc), desc_format, XMULT_ARGS(percent));
    return print_desc(desc, dest_rect);
}

static int lucky_cat_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] =
        TTE_RED_TAG "X0.25" TTE_BLACK_TAG " множ. за каждое срабатывание " TTE_YELLOW_TAG "Счастливой" TTE_BLACK_TAG " карты\n\n(Сейчас " TTE_RED_TAG XMULT_FMT TTE_BLACK_TAG " множ.)";
    char desc[180];
    int percent = joker_cards_xmult_percent(joker->id, joker->persistent_state, 0);
    snprintf(desc, sizeof(desc), desc_format, XMULT_ARGS(percent));
    return print_desc(desc, dest_rect);
}

static int vampire_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] =
        TTE_RED_TAG "X0.1" TTE_BLACK_TAG " множ. за каждое улучшение сыгранных карт, улучшение снимается\n\n(Сейчас " TTE_RED_TAG XMULT_FMT TTE_BLACK_TAG " множ.)";
    char desc[200];
    int percent = joker_cards_xmult_percent(joker->id, joker->persistent_state, 0);
    snprintf(desc, sizeof(desc), desc_format, XMULT_ARGS(percent));
    return print_desc(desc, dest_rect);
}

static int midas_mask_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Сыгранные " TTE_YELLOW_TAG "картинки" TTE_BLACK_TAG " становятся " TTE_YELLOW_TAG "Золотыми";
    return print_desc(desc, dest_rect);
}

static int cartomancer_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Создаёт карту " TTE_PURPLE_TAG "Таро" TTE_BLACK_TAG " при выборе блайнда\n(нужен свободный слот)";
    return print_desc(desc, dest_rect);
}

static int constellation_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] =
        TTE_RED_TAG "X0.1" TTE_BLACK_TAG " множ. за каждую использованную " TTE_YELLOW_TAG "Планету" TTE_BLACK_TAG "\n\n(Сейчас " TTE_RED_TAG XMULT_FMT TTE_BLACK_TAG " множ.)";
    char desc[180];
    int percent = joker_cards_xmult_percent(joker->id, joker->persistent_state, 0);
    snprintf(desc, sizeof(desc), desc_format, XMULT_ARGS(percent));
    return print_desc(desc, dest_rect);
}

static int fortune_teller_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] =
        TTE_RED_TAG "+1" TTE_BLACK_TAG " множ. за каждую использованную " TTE_PURPLE_TAG "Таро" TTE_BLACK_TAG " за забег\n\n(Сейчас " TTE_RED_TAG "+%d" TTE_BLACK_TAG " множ.)";
    char desc[180];
    snprintf(desc, sizeof(desc), desc_format, joker_cards_fortune_teller_mult(g_game_vars.tarots_used));
    return print_desc(desc, dest_rect);
}

static int hallucination_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_GREEN_TAG "1 из 2" TTE_BLACK_TAG ", что при открытии бустера создастся " TTE_PURPLE_TAG "Таро" TTE_BLACK_TAG "\n(нужен свободный слот)";
    return print_desc(desc, dest_rect);
}

static int stone_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc_format[] =
        TTE_BLUE_TAG "+25" TTE_BLACK_TAG " фишек за каждую " TTE_YELLOW_TAG "Каменную" TTE_BLACK_TAG " карту в колоде\n\n(Сейчас " TTE_BLUE_TAG "+%d" TTE_BLACK_TAG " фишек)";
    char desc[180];
    snprintf(desc, sizeof(desc), desc_format, joker_cards_stone_chips(stone_joker_stone_count()));
    return print_desc(desc, dest_rect);
}

static int marble_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Добавляет в колоду одну " TTE_YELLOW_TAG "Каменную" TTE_BLACK_TAG " карту при выборе блайнда";
    return print_desc(desc, dest_rect);
}

static int certificate_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "В начале раунда в руку попадает случайная карта со случайной " TTE_YELLOW_TAG "печатью";
    return print_desc(desc, dest_rect);
}

static int space_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_GREEN_TAG "1 из 4" TTE_BLACK_TAG ", что сыгранная рука повысит " TTE_YELLOW_TAG "уровень" TTE_BLACK_TAG " на 1";
    return print_desc(desc, dest_rect);
}

static int hiker_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Каждая сыгранная карта навсегда получает " TTE_BLUE_TAG "+5" TTE_BLACK_TAG " фишек при подсчёте";
    return print_desc(desc, dest_rect);
}

static int burglar_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "При выборе блайнда " TTE_RED_TAG "+3" TTE_BLACK_TAG " руки и " TTE_RED_TAG "все сбросы пропадают";
    return print_desc(desc, dest_rect);
}

static int pareidolia_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] = TTE_BLACK_TAG "Все карты считаются " TTE_YELLOW_TAG "картинками";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int shortcut_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Позволяет собирать " TTE_YELLOW_TAG "Стриты" TTE_BLACK_TAG
                      " с пропусками в " TTE_YELLOW_TAG "1 ранг" TTE_BLACK_TAG
                      "\n\n(напр.: " TTE_YELLOW_TAG "10 8 6 5 3" TTE_BLACK_TAG ")";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static int four_fingers_joker_desc(Joker* joker, Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "Все " TTE_YELLOW_TAG "Флеши" TTE_BLACK_TAG " и " TTE_YELLOW_TAG
                      "Стриты" TTE_BLACK_TAG " можно собрать из 4 карт";
    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

#pragma endregion

#pragma region JOKER EFFECTS

static u32 joker_effect_noop(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    return JOKER_EFFECT_FLAG_NONE;
}

static u32 default_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)
    *joker_effect = &s_shared_joker_effect;

    (*joker_effect)->mult = 4;

    return JOKER_EFFECT_FLAG_MULT;
}

static u32 sinful_joker_effect(
    Card* scored_card,
    u8 sinful_suit,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY_WITH_CARD(scored_card, JOKER_EVENT_ON_CARD_SCORED, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (card_has_suit(scored_card, sinful_suit))
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->mult = 3;
        effect_flags_ret = JOKER_EFFECT_FLAG_MULT;
    }
    return effect_flags_ret;
}

static u32 greedy_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    return sinful_joker_effect(scored_card, DIAMONDS, joker_event, joker_effect);
}

static u32 lusty_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    return sinful_joker_effect(scored_card, HEARTS, joker_event, joker_effect);
}

static u32 wrathful_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    return sinful_joker_effect(scored_card, SPADES, joker_event, joker_effect);
}

static u32 gluttonous_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    return sinful_joker_effect(scored_card, CLUBS, joker_event, joker_effect);
}

static u32 jolly_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (get_contained_hands()->PAIR)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->mult = 8;
        effect_flags_ret = JOKER_EFFECT_FLAG_MULT;
    }

    return effect_flags_ret;
}

static u32 zany_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (get_contained_hands()->THREE_OF_A_KIND)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->mult = 12;
        effect_flags_ret = JOKER_EFFECT_FLAG_MULT;
    }

    return effect_flags_ret;
}

static u32 mad_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (get_contained_hands()->TWO_PAIR)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->mult = 10;
        effect_flags_ret = JOKER_EFFECT_FLAG_MULT;
    }

    return effect_flags_ret;
}

static u32 crazy_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (get_contained_hands()->STRAIGHT)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->mult = 12;
        effect_flags_ret = JOKER_EFFECT_FLAG_MULT;
    }

    return effect_flags_ret;
}

static u32 droll_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (get_contained_hands()->FLUSH)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->mult = 10;
        effect_flags_ret = JOKER_EFFECT_FLAG_MULT;
    }

    return effect_flags_ret;
}

static u32 sly_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (get_contained_hands()->PAIR)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->chips = 50;
        effect_flags_ret = JOKER_EFFECT_FLAG_CHIPS;
    }

    return effect_flags_ret;
}

static u32 wily_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (get_contained_hands()->THREE_OF_A_KIND)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->chips = 100;
        effect_flags_ret = JOKER_EFFECT_FLAG_CHIPS;
    }

    return effect_flags_ret;
}

static u32 clever_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (get_contained_hands()->TWO_PAIR)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->chips = 80;
        effect_flags_ret = JOKER_EFFECT_FLAG_CHIPS;
    }

    return effect_flags_ret;
}

static u32 devious_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (get_contained_hands()->STRAIGHT)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->chips = 100;
        effect_flags_ret = JOKER_EFFECT_FLAG_CHIPS;
    }

    return effect_flags_ret;
}

static u32 crafty_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (get_contained_hands()->FLUSH)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->chips = 80;
        effect_flags_ret = JOKER_EFFECT_FLAG_CHIPS;
    }

    return effect_flags_ret;
}

static u32 half_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    int played_size = get_played_size();
    if (played_size <= 3)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->mult = 20;
        effect_flags_ret = JOKER_EFFECT_FLAG_MULT;
    }

    return effect_flags_ret;
}

static u32 stencil_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    *joker_effect = &s_shared_joker_effect;

    List* jokers = get_jokers_list();

    // +1 xmult per empty joker slot...
    int num_jokers = list_get_len(jokers);

    (*joker_effect)->xmult = g_game_vars.joker_slots - num_jokers;

    // ...and also each stencil_joker adds +1 xmult
    ListItr itr = list_itr_create(jokers);
    JokerObject* joker_object;

    while ((joker_object = list_itr_next(&itr)))
    {
        if (joker_object->joker->id == STENCIL_JOKER_ID)
            (*joker_effect)->xmult++;
    }

    return JOKER_EFFECT_FLAG_XMULT;
}

#define MISPRINT_MAX_MULT 23
static u32 misprint_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    *joker_effect = &s_shared_joker_effect;

    (*joker_effect)->mult = rng_get_u32(RNG_SEQ_JOKER_MISPRINT) % (MISPRINT_MAX_MULT + 1);

    return JOKER_EFFECT_FLAG_MULT;
}

static u32 walkie_talkie_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY_WITH_CARD(scored_card, JOKER_EVENT_ON_CARD_SCORED, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (card_effective_rank(scored_card) == TEN || card_effective_rank(scored_card) == FOUR)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->chips = 10;
        (*joker_effect)->mult = 4;
        effect_flags_ret = JOKER_EFFECT_FLAG_CHIPS | JOKER_EFFECT_FLAG_MULT;
    }

    return effect_flags_ret;
}

static u32 fibonnaci_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY_WITH_CARD(scored_card, JOKER_EVENT_ON_CARD_SCORED, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    switch (card_effective_rank(scored_card))
    {
        case ACE:
        case TWO:
        case THREE:
        case FIVE:
        case EIGHT:
            *joker_effect = &s_shared_joker_effect;
            (*joker_effect)->mult = 8;
            effect_flags_ret = JOKER_EFFECT_FLAG_MULT;
            break;
        default:
            break;
    }

    return effect_flags_ret;
}

static u32 banner_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (get_num_discards_remaining() > 0)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->chips = 30 * get_num_discards_remaining();
        effect_flags_ret = JOKER_EFFECT_FLAG_CHIPS;
    }

    return effect_flags_ret;
}

static u32 mystic_summit_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (get_num_discards_remaining() == 0)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->mult = 15;
        effect_flags_ret = JOKER_EFFECT_FLAG_MULT;
    }

    return effect_flags_ret;
}

static u32 blackboard_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    bool all_cards_are_spades_or_clubs = true;
    CardObject** hand = get_hand_array();
    for (int i = 0; i < g_game_vars.hand_size; i++)
    {
        Card* card = hand[i]->card;
        if (!card_has_suit(card, SPADES) && !card_has_suit(card, CLUBS))
        {
            all_cards_are_spades_or_clubs = false;
            break;
        }
    }

    if (all_cards_are_spades_or_clubs)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->xmult = 3;
        effect_flags_ret = JOKER_EFFECT_FLAG_XMULT;
    }

    return effect_flags_ret;
}

static u32 blue_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    *joker_effect = &s_shared_joker_effect;

    (*joker_effect)->chips = (get_deck_top() + 1) * 2;

    return JOKER_EFFECT_FLAG_CHIPS;
}

static u32 raised_fist_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    s32* p_lowest_value_index = &(joker->scoring_state);

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    switch (joker_event)
    {
        // Use this event to compute the index of the lowest value card only once.
        // Aces are always considered high value, even in an ace-low straight
        case JOKER_EVENT_ON_HAND_PLAYED:
            // index initialized at 0 but accessed only if
            // hand_size > 0 so we're never out of bounds
            *p_lowest_value_index = 0;
            u8 lowest_value = IMPOSSIBLY_HIGH_CARD_VALUE;
            CardObject** hand = get_hand_array();
            for (int i = 0; i < g_game_vars.hand_size; i++)
            {
                u8 value = card_get_value(hand[i]->card);
                if (lowest_value > value)
                {
                    *p_lowest_value_index = i;
                    lowest_value = value;
                }
            }
            break;

        case JOKER_EVENT_ON_CARD_HELD:
            if (get_scored_card_index() == *p_lowest_value_index)
            {
                *joker_effect = &s_shared_joker_effect;

                (*joker_effect)->mult = 2 * card_get_value(scored_card);
                effect_flags_ret = JOKER_EFFECT_FLAG_MULT;
            }
            break;

        default:
            break;
    }

    return effect_flags_ret;
}

static u32 reserved_parking_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_ON_CARD_HELD, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (card_is_face(scored_card) && (rng_get_u32(RNG_SEQ_JOKER_RESERVED_PARKING) % 2 == 0))
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->money = 1;
        effect_flags_ret = JOKER_EFFECT_FLAG_MONEY;
    }

    return effect_flags_ret;
};

static u32 business_card_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY_WITH_CARD(scored_card, JOKER_EVENT_ON_CARD_SCORED, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (card_is_face(scored_card) && (rng_get_u32(RNG_SEQ_JOKER_BUSINESS_CARD) % 2 == 0))
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->money = 2;
        effect_flags_ret = JOKER_EFFECT_FLAG_MONEY;
    }

    return effect_flags_ret;
}

static u32 scholar_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY_WITH_CARD(scored_card, JOKER_EVENT_ON_CARD_SCORED, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (card_effective_rank(scored_card) == ACE)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->chips = 20;
        (*joker_effect)->mult = 4;
        effect_flags_ret = JOKER_EFFECT_FLAG_CHIPS | JOKER_EFFECT_FLAG_MULT;
    }

    return effect_flags_ret;
}

static u32 scary_face_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY_WITH_CARD(scored_card, JOKER_EVENT_ON_CARD_SCORED, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (card_is_face(scored_card))
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->chips = 30;
        effect_flags_ret = JOKER_EFFECT_FLAG_CHIPS;
    }

    return effect_flags_ret;
}

static u32 abstract_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    *joker_effect = &s_shared_joker_effect;

    // +1 xmult per occupied joker slot
    int num_jokers = list_get_len(get_jokers_list());

    (*joker_effect)->mult = num_jokers * 3;

    return JOKER_EFFECT_FLAG_MULT;
}

static u32 bull_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    // The wiki says it does nothing if money is 0 or below
    // This allows us to avoid scoring negative Chips
    if (g_game_vars.money > 0)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->chips = g_game_vars.money * 2;
        effect_flags_ret = JOKER_EFFECT_FLAG_CHIPS;
    }

    return effect_flags_ret;
}

static u32 smiley_face_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY_WITH_CARD(scored_card, JOKER_EVENT_ON_CARD_SCORED, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (card_is_face(scored_card))
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->mult = 5;
        effect_flags_ret = JOKER_EFFECT_FLAG_MULT;
    }

    return effect_flags_ret;
}

static u32 even_steven_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY_WITH_CARD(scored_card, JOKER_EVENT_ON_CARD_SCORED, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    switch (card_effective_rank(scored_card))
    {
        case KING:
        case QUEEN:
        case JACK:
            break;
        default:
            if (card_get_value(scored_card) % 2 == 0)
            {
                *joker_effect = &s_shared_joker_effect;

                (*joker_effect)->mult = 4;
                effect_flags_ret = JOKER_EFFECT_FLAG_MULT;
            }
            break;
    }

    return effect_flags_ret;
}

static u32 odd_todd_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY_WITH_CARD(scored_card, JOKER_EVENT_ON_CARD_SCORED, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (card_get_value(scored_card) % 2 == 1) // todo test ace
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->chips = 31;
        effect_flags_ret = JOKER_EFFECT_FLAG_CHIPS;
    }

    return effect_flags_ret;
}

static u32 acrobat_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    // 0 remaining hands mean we're scoring the last hand
    if (get_num_hands_remaining() == 0)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->xmult = 3;
        effect_flags_ret = JOKER_EFFECT_FLAG_XMULT;
    }

    return effect_flags_ret;
}

static u32 hanging_chad_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;
    s32* p_remaining_retriggers = &(joker->scoring_state);

    switch (joker_event)
    {
        case JOKER_EVENT_ON_HAND_PLAYED:
            *p_remaining_retriggers = 2;
            break;

        // No need to check if this is the first card scored or not
        // p_remaining_retriggers will always reach 0 on the first card, then retrigger
        // will be false and scoring will go onto the next card
        case JOKER_EVENT_ON_CARD_SCORED_END:
            *joker_effect = &s_shared_joker_effect;

            (*joker_effect)->retrigger = (*p_remaining_retriggers > 0);
            if ((*joker_effect)->retrigger)
            {
                *p_remaining_retriggers -= 1;
                (*joker_effect)->message = "Снова!";
                effect_flags_ret = JOKER_EFFECT_FLAG_RETRIGGER | JOKER_EFFECT_FLAG_MESSAGE;
            }
            break;

        default:
            break;
    }

    return effect_flags_ret;
}

static u32 the_duo_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (get_contained_hands()->PAIR)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->xmult = 2;
        effect_flags_ret = JOKER_EFFECT_FLAG_XMULT;
    }

    return effect_flags_ret;
}

static u32 the_trio_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (get_contained_hands()->THREE_OF_A_KIND)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->xmult = 3;
        effect_flags_ret = JOKER_EFFECT_FLAG_XMULT;
    }

    return effect_flags_ret;
}

static u32 the_family_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (get_contained_hands()->FOUR_OF_A_KIND)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->xmult = 4;
        effect_flags_ret = JOKER_EFFECT_FLAG_XMULT;
    }

    return effect_flags_ret;
}

static u32 the_order_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (get_contained_hands()->STRAIGHT)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->xmult = 3;
        effect_flags_ret = JOKER_EFFECT_FLAG_XMULT;
    }

    return effect_flags_ret;
}

static u32 the_tribe_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (get_contained_hands()->FLUSH)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->xmult = 2;
        effect_flags_ret = JOKER_EFFECT_FLAG_XMULT;
    }

    return effect_flags_ret;
}

static u32 bootstraps_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    // Same protection as the Bull Joker
    if (g_game_vars.money > 0)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->mult = (g_game_vars.money / 5) * 2;
        effect_flags_ret = JOKER_EFFECT_FLAG_MULT;
    }

    return effect_flags_ret;
}

static u32 shoot_the_moon_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_ON_CARD_HELD, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (card_effective_rank(scored_card) == QUEEN)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->mult = 13;
        effect_flags_ret = JOKER_EFFECT_FLAG_MULT;
    }

    return effect_flags_ret;
}

static u32 photograph_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    s32* p_first_face_index = &(joker->scoring_state);

    switch (joker_event)
    {
        case JOKER_EVENT_ON_HAND_PLAYED:
            *p_first_face_index = UNDEFINED;
            break;

        case JOKER_EVENT_ON_CARD_SCORED:
            // has a face card been encountered already, and if not, is the current scoring card a
            // face card?
            if (*p_first_face_index == UNDEFINED && card_is_face(scored_card))
            {
                *p_first_face_index = get_scored_card_index();
            }
            // if we have a face card index saved, check against it and give mult accordingly
            // Doing this now will trigger the effect the first time we encounter the face card,
            // and we will catch potential retriggers
            if (*p_first_face_index == get_scored_card_index())
            {
                *joker_effect = &s_shared_joker_effect;

                (*joker_effect)->xmult = 2;
                effect_flags_ret = JOKER_EFFECT_FLAG_XMULT;
            }
            break;
        default:
            break;
    }

    return effect_flags_ret;
}

static u32 dusk_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    s32* p_last_retriggered_index = &(joker->scoring_state);

    switch (joker_event)
    {
        case JOKER_EVENT_ON_HAND_PLAYED:
            // start at -1 so that a first index of 0 can satisfy the retrigger condition below
            *p_last_retriggered_index = UNDEFINED;
            break;

        case JOKER_EVENT_ON_CARD_SCORED_END:
            // Only retrigger current card if it's strictly after the last one we retriggered
            if (get_num_hands_remaining() == 0)
            {
                *joker_effect = &s_shared_joker_effect;

                (*joker_effect)->retrigger = (*p_last_retriggered_index < get_scored_card_index());
                if ((*joker_effect)->retrigger)
                {
                    *p_last_retriggered_index = get_scored_card_index();
                    (*joker_effect)->message = "Снова!";
                    effect_flags_ret = JOKER_EFFECT_FLAG_RETRIGGER | JOKER_EFFECT_FLAG_MESSAGE;
                }
            }

            break;

        default:
            break;
    }

    return effect_flags_ret;
}

static u32 blueprint_brainstorm_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    // No need for this kind of init since these Jokers
    // will have their data copied when needed
    if (joker_event == JOKER_EVENT_ON_JOKER_CREATED ||
        joker_event == JOKER_EVENT_ON_HAND_SCORED_END || joker_event == JOKER_EVENT_ON_ROUND_END)
    {
        return effect_flags_ret;
    }

    // find ourselves in the Jokers list
    List* jokers = get_jokers_list();
    ListItr itr = list_itr_create(jokers);
    JokerObject* copied_joker_object;
    while ((copied_joker_object = list_itr_next(&itr)))
    {
        if (copied_joker_object->joker == joker)
        {
            break;
        }
    }

    // This shouldn't happen since if we are a scoring Joker, we should always
    // be part of the Jokers list, but being extra careful doesn't cost much
    if (copied_joker_object == NULL)
    {
        return effect_flags_ret;
    }

    // find the copied Joker, may need to bounce around Blueprints and a Brainstorm
    // If we encounter NULL, we have a Blueprint at the end of the list that can't copy anything.
    // If we go through a Brainstorms twice, we will be in a loop and need to exit
    u8 brainstorm_counter = 0;
    do
    {
        switch (copied_joker_object->joker->id)
        {
            // get the next Joker for Blueprint
            case BLUEPRINT_JOKER_ID:
                copied_joker_object = list_itr_next(&itr);
                break;

            // Get the first (leftmost) Joker for Brainstorm
            case BRAINSTORM_JOKER_ID:
                brainstorm_counter++;
                itr = list_itr_create(jokers);
                copied_joker_object = list_itr_next(&itr);
                break;

            // We encountered a Joker that isn't a Copying Joker and copy it now
            // but how we copy it depends on this Joker's ID because they don't
            // all handle data the same way.
            default:
                u8 copied_joker_id = copied_joker_object->joker->id;
                const JokerInfo* copied_joker_info = get_joker_registry_entry(copied_joker_id);

                // Copy the persistent data
                joker->persistent_state = copied_joker_object->joker->persistent_state;

                // Then regardless of if we copied the data above, apply the
                // copied JokerEffect function to the local data
                effect_flags_ret =
                    copied_joker_info
                        ->joker_effect_func(joker, scored_card, joker_event, joker_effect);

                // make also sure we don't expire
                effect_flags_ret &= ~JOKER_EFFECT_FLAG_EXPIRE;

                // exit the loop
                copied_joker_object = NULL;

                break;
        }
    } while (copied_joker_object != NULL && brainstorm_counter < 2);

    return effect_flags_ret;
}

static u32 hack_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    s32* p_last_retriggered_index = &(joker->scoring_state);

    switch (joker_event)
    {
        case JOKER_EVENT_ON_HAND_PLAYED:
            *p_last_retriggered_index = UNDEFINED;
            break;

        case JOKER_EVENT_ON_CARD_SCORED_END:
            // Works the same way as Dusk, but check what rank the card is
            switch (card_effective_rank(scored_card))
            {
                case TWO:
                case THREE:
                case FOUR:
                case FIVE:
                    *joker_effect = &s_shared_joker_effect;

                    (*joker_effect)->retrigger =
                        (*p_last_retriggered_index < get_scored_card_index());
                    if ((*joker_effect)->retrigger)
                    {
                        *p_last_retriggered_index = get_scored_card_index();
                        (*joker_effect)->message = "Снова!";
                        effect_flags_ret = JOKER_EFFECT_FLAG_RETRIGGER | JOKER_EFFECT_FLAG_MESSAGE;
                    }
                    break;
            }
            break;

        default:
            break;
    }

    return effect_flags_ret;
}

static u32 seltzer_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    s32* p_last_retriggered_idx = &(joker->scoring_state);
    s32* p_hands_left_until_exp = &(joker->persistent_state);

    switch (joker_event)
    {
        case JOKER_EVENT_ON_JOKER_CREATED:
            *p_hands_left_until_exp = 10; // remaining retriggered hands
            break;

        case JOKER_EVENT_ON_HAND_PLAYED:
            *p_last_retriggered_idx = UNDEFINED;
            break;

        case JOKER_EVENT_ON_CARD_SCORED_END:
            // Works the same way as Dusk
            // No need to check for p_hands_left_until_exp because the Joker
            // will be destroyed the moment we hit 0
            *joker_effect = &s_shared_joker_effect;

            (*joker_effect)->retrigger = ((*p_last_retriggered_idx) < get_scored_card_index());
            if ((*joker_effect)->retrigger)
            {
                *p_last_retriggered_idx = get_scored_card_index();
                (*joker_effect)->message = "Снова!";
                effect_flags_ret = JOKER_EFFECT_FLAG_RETRIGGER | JOKER_EFFECT_FLAG_MESSAGE;
            }
            break;

        case JOKER_EVENT_ON_HAND_SCORED_END:
            *joker_effect = &s_shared_joker_effect;
            effect_flags_ret = JOKER_EFFECT_FLAG_MESSAGE;

            (*p_hands_left_until_exp)--;
            if (*p_hands_left_until_exp > 0)
            {
                // Need to do this for now because the message's memory can't really be allocated
                // So we can't use snprintf to craft a message depending on the number of hands left
                static const char* SELTZER_MESSAGES[] =
                    {"1", "2", "3", "4", "5", "6", "7", "8", "9"};
                (*joker_effect)->message = (char*)SELTZER_MESSAGES[(*p_hands_left_until_exp) - 1];
            }
            else
            {
                (*joker_effect)->message = "Выпито!";
                (*joker_effect)->expire = true;
                effect_flags_ret |= JOKER_EFFECT_FLAG_EXPIRE;
            }
            break;

        default:
            break;
    }

    return effect_flags_ret;
}

static u32 sock_and_buskin_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    s32* p_last_retriggered_face_index = &(joker->scoring_state);

    switch (joker_event)
    {
        case JOKER_EVENT_ON_HAND_PLAYED:
            *p_last_retriggered_face_index = UNDEFINED;
            break;

        case JOKER_EVENT_ON_CARD_SCORED_END:
            *joker_effect = &s_shared_joker_effect;

            // Works the same way as Dusk, but for face cards
            (*joker_effect)->retrigger =
                ((*p_last_retriggered_face_index < get_scored_card_index()) &&
                 card_is_face(scored_card));
            if ((*joker_effect)->retrigger)
            {
                *p_last_retriggered_face_index = get_scored_card_index();
                (*joker_effect)->message = "Снова!";
                effect_flags_ret = JOKER_EFFECT_FLAG_RETRIGGER | JOKER_EFFECT_FLAG_MESSAGE;
            }
            break;

        default:
            break;
    }

    return effect_flags_ret;
}

static u32 baron_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY_WITH_CARD(scored_card, JOKER_EVENT_ON_CARD_HELD, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    // X1.5 in fixed point (applied in round.c, rounded to the nearest integer mult)
    if (card_effective_rank(scored_card) == KING)
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->xmult_fx = FIX_ONE * 3 / 2;
        effect_flags_ret = JOKER_EFFECT_FLAG_XMULT_FX;
    }

    return effect_flags_ret;
}

static u32 ride_the_bus_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    switch (joker_event)
    {
        case JOKER_EVENT_ON_HAND_PLAYED:
            joker->persistent_state++;
            break;

        case JOKER_EVENT_ON_CARD_SCORED:
            if (card_is_face(scored_card))
            {
                joker->persistent_state = 0;
            }
            break;

        case JOKER_EVENT_INDEPENDENT:
            if (joker->persistent_state > 0)
            {
                *joker_effect = &s_shared_joker_effect;

                (*joker_effect)->mult = joker->persistent_state;
                effect_flags_ret = JOKER_EFFECT_FLAG_MULT;
            }
            break;

        default:
            break;
    }

    return effect_flags_ret;
}

static u32 supernova_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    enum HandType hand_type = get_hand_type();
    if (hand_type != NONE)
    {
        *joker_effect = &s_shared_joker_effect;

        // The hand that is being played has already been counted
        u32 played = g_game_vars.nb_played_hands[hand_type - 1];
        // A Royal Flush is a Straight Flush for the play counter, like in the original game
        if (hand_type == STRAIGHT_FLUSH)
            played += g_game_vars.nb_played_hands[ROYAL_FLUSH - 1];
        else if (hand_type == ROYAL_FLUSH)
            played += g_game_vars.nb_played_hands[STRAIGHT_FLUSH - 1];

        (*joker_effect)->mult = played;
        effect_flags_ret = JOKER_EFFECT_FLAG_MULT;
    }

    return effect_flags_ret;
}

static u32 runner_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    switch (joker_event)
    {
        case JOKER_EVENT_ON_HAND_PLAYED:
            if (get_contained_hands()->STRAIGHT)
            {
                joker->persistent_state += 15;
            }
            break;

        case JOKER_EVENT_INDEPENDENT:
            if (joker->persistent_state > 0)
            {
                *joker_effect = &s_shared_joker_effect;

                (*joker_effect)->chips = joker->persistent_state;
                effect_flags_ret = JOKER_EFFECT_FLAG_CHIPS;
            }
            break;

        default:
            break;
    }

    return effect_flags_ret;
}

static u32 square_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    switch (joker_event)
    {
        case JOKER_EVENT_ON_HAND_PLAYED:
            if (get_played_size() == 4)
            {
                joker->persistent_state += 4;
            }
            break;

        case JOKER_EVENT_INDEPENDENT:
            if (joker->persistent_state > 0)
            {
                *joker_effect = &s_shared_joker_effect;

                (*joker_effect)->chips = joker->persistent_state;
                effect_flags_ret = JOKER_EFFECT_FLAG_CHIPS;
            }
            break;

        default:
            break;
    }

    return effect_flags_ret;
}

static u32 spare_trousers_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    switch (joker_event)
    {
        case JOKER_EVENT_ON_HAND_PLAYED:
            if (get_contained_hands()->TWO_PAIR)
            {
                joker->persistent_state += 2;
            }
            break;

        case JOKER_EVENT_INDEPENDENT:
            if (joker->persistent_state > 0)
            {
                *joker_effect = &s_shared_joker_effect;

                (*joker_effect)->mult = joker->persistent_state;
                effect_flags_ret = JOKER_EFFECT_FLAG_MULT;
            }
            break;

        default:
            break;
    }

    return effect_flags_ret;
}

static u32 wee_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    switch (joker_event)
    {
        case JOKER_EVENT_ON_CARD_SCORED:
            if (card_effective_rank(scored_card) == TWO)
            {
                joker->persistent_state += 8;
            }
            break;

        case JOKER_EVENT_INDEPENDENT:
            if (joker->persistent_state > 0)
            {
                *joker_effect = &s_shared_joker_effect;

                (*joker_effect)->chips = joker->persistent_state;
                effect_flags_ret = JOKER_EFFECT_FLAG_CHIPS;
            }
            break;

        default:
            break;
    }

    return effect_flags_ret;
}

static u32 ice_cream_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    s32* p_chips = &(joker->persistent_state);

    switch (joker_event)
    {
        case JOKER_EVENT_ON_JOKER_CREATED:
            *p_chips = 100;
            break;

        case JOKER_EVENT_INDEPENDENT:
            if (*p_chips > 0)
            {
                *joker_effect = &s_shared_joker_effect;

                (*joker_effect)->chips = *p_chips;
                effect_flags_ret = JOKER_EFFECT_FLAG_CHIPS;
            }
            break;

        case JOKER_EVENT_ON_HAND_SCORED_END:
            *joker_effect = &s_shared_joker_effect;
            effect_flags_ret = JOKER_EFFECT_FLAG_MESSAGE;

            *p_chips -= 5;
            if (*p_chips > 0)
            {
                (*joker_effect)->message = "-5";
            }
            else
            {
                (*joker_effect)->message = "Растаяло!";
                (*joker_effect)->expire = true;
                effect_flags_ret |= JOKER_EFFECT_FLAG_EXPIRE;
            }
            break;

        default:
            break;
    }

    return effect_flags_ret;
}

static u32 loyalty_card_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    // persistent_state counts the hands played since the last bonus, 0 after the 6th hand.
    // It is only 0 while a hand is being scored right after a bonus hand, so the bonus
    // doesn't depend on the data of the Blueprint or Brainstorm that copies this Joker.
    s32* p_hands = &(joker->persistent_state);

    switch (joker_event)
    {
        case JOKER_EVENT_ON_HAND_PLAYED:
            *p_hands = (*p_hands + 1) % 6;
            break;

        case JOKER_EVENT_INDEPENDENT:
            if (*p_hands == 0)
            {
                *joker_effect = &s_shared_joker_effect;

                (*joker_effect)->xmult = 4;
                effect_flags_ret = JOKER_EFFECT_FLAG_XMULT;
            }
            break;

        default:
            break;
    }

    return effect_flags_ret;
}

static u32 onyx_agate_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY_WITH_CARD(scored_card, JOKER_EVENT_ON_CARD_SCORED, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (card_has_suit(scored_card, CLUBS))
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->mult = 7;
        effect_flags_ret = JOKER_EFFECT_FLAG_MULT;
    }

    return effect_flags_ret;
}

static u32 arrowhead_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY_WITH_CARD(scored_card, JOKER_EVENT_ON_CARD_SCORED, joker_event)

    u32 effect_flags_ret = JOKER_EFFECT_FLAG_NONE;

    if (card_has_suit(scored_card, SPADES))
    {
        *joker_effect = &s_shared_joker_effect;

        (*joker_effect)->chips = 50;
        effect_flags_ret = JOKER_EFFECT_FLAG_CHIPS;
    }

    return effect_flags_ret;
}

static u32 rocket_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    // The payout is read from the persistent state when the round ends
    if (joker_event == JOKER_EVENT_ON_JOKER_CREATED)
        joker->persistent_state = ROCKET_START_MONEY;

    return JOKER_EFFECT_FLAG_NONE;
}

static u32 stuntman_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    *joker_effect = &s_shared_joker_effect;

    (*joker_effect)->chips = 250;
    return JOKER_EFFECT_FLAG_CHIPS;
}

static u32 swashbuckler_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    int mult = swashbuckler_get_mult(joker);
    if (mult <= 0)
        return JOKER_EFFECT_FLAG_NONE;

    *joker_effect = &s_shared_joker_effect;

    (*joker_effect)->mult = mult;
    return JOKER_EFFECT_FLAG_MULT;
}

static u32 golden_ticket_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY_WITH_CARD(scored_card, JOKER_EVENT_ON_CARD_SCORED, joker_event)

    if (scored_card->enhancement != CARD_ENHANCEMENT_GOLD)
        return JOKER_EFFECT_FLAG_NONE;

    *joker_effect = &s_shared_joker_effect;

    (*joker_effect)->money = 4;
    return JOKER_EFFECT_FLAG_MONEY;
}

static u32 rough_gem_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY_WITH_CARD(scored_card, JOKER_EVENT_ON_CARD_SCORED, joker_event)

    if (!card_has_suit(scored_card, DIAMONDS))
        return JOKER_EFFECT_FLAG_NONE;

    *joker_effect = &s_shared_joker_effect;

    (*joker_effect)->money = 1;
    return JOKER_EFFECT_FLAG_MONEY;
}

// Applies an "X" multiplier given as a percent by adding the difference to the mult
static u32 joker_percent_xmult_effect(int percent, JokerEffect** joker_effect)
{
    u32 gain = joker_cards_mult_gain(g_game_vars.mult, percent);
    if (gain == 0)
        return JOKER_EFFECT_FLAG_NONE;

    *joker_effect = &s_shared_joker_effect;
    (*joker_effect)->mult = gain;
    return JOKER_EFFECT_FLAG_MULT;
}

static u32 steel_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    int percent = joker_cards_xmult_percent(joker->id, 0, steel_joker_steel_count());
    return joker_percent_xmult_effect(percent, joker_effect);
}

// Glass Joker, Lucky Cat and Constellation: X mult from the counter kept in persistent_state
static u32 glass_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    int percent = joker_cards_xmult_percent(joker->id, joker->persistent_state, 0);
    return joker_percent_xmult_effect(percent, joker_effect);
}

static u32 lucky_cat_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    return glass_joker_effect(joker, scored_card, joker_event, joker_effect);
}

static u32 constellation_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    return glass_joker_effect(joker, scored_card, joker_event, joker_effect);
}

// Redraws the sprite of a played card after its enhancement changed
static void played_card_refresh(int played_idx)
{
    card_object_set_sprite(get_played_array()[played_idx], MAX_HAND_SIZE + played_idx);
}

static u32 vampire_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    if (joker_event == JOKER_EVENT_INDEPENDENT)
    {
        int percent = joker_cards_xmult_percent(joker->id, joker->persistent_state, 0);
        return joker_percent_xmult_effect(percent, joker_effect);
    }

    if (joker_event != JOKER_EVENT_ON_HAND_PLAYED)
        return JOKER_EFFECT_FLAG_NONE;

    // Before the hand is scored the scoring cards lose their enhancement and feed the Vampire
    CardObject** played = get_played_array();
    int eaten = 0;
    for (int i = 0; i <= get_played_top(); i++)
    {
        if (played[i] == NULL || !card_object_is_selected(played[i]) || played[i]->card->debuffed)
            continue;
        if (joker_cards_vampire_eat(played[i]->card))
        {
            eaten++;
            played_card_refresh(i);
        }
    }

    if (eaten == 0)
        return JOKER_EFFECT_FLAG_NONE;

    joker->persistent_state += eaten;
    *joker_effect = &s_shared_joker_effect;
    (*joker_effect)->message = "Ням!";
    return JOKER_EFFECT_FLAG_MESSAGE;
}

static u32 midas_mask_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_ON_HAND_PLAYED, joker_event)

    CardObject** played = get_played_array();
    int gilded = 0;
    for (int i = 0; i <= get_played_top(); i++)
    {
        if (played[i] == NULL || !card_object_is_selected(played[i]) || played[i]->card->debuffed)
            continue;
        if (joker_cards_midas_gild(played[i]->card, card_is_face(played[i]->card)))
        {
            gilded++;
            played_card_refresh(i);
        }
    }

    if (gilded == 0)
        return JOKER_EFFECT_FLAG_NONE;

    *joker_effect = &s_shared_joker_effect;
    (*joker_effect)->message = "Золото!";
    return JOKER_EFFECT_FLAG_MESSAGE;
}

static u32 fortune_teller_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    int mult = joker_cards_fortune_teller_mult(g_game_vars.tarots_used);
    if (mult <= 0)
        return JOKER_EFFECT_FLAG_NONE;

    *joker_effect = &s_shared_joker_effect;
    (*joker_effect)->mult = mult;
    return JOKER_EFFECT_FLAG_MULT;
}

static u32 stone_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_INDEPENDENT, joker_event)

    int chips = joker_cards_stone_chips(stone_joker_stone_count());
    if (chips <= 0)
        return JOKER_EFFECT_FLAG_NONE;

    *joker_effect = &s_shared_joker_effect;
    (*joker_effect)->chips = chips;
    return JOKER_EFFECT_FLAG_CHIPS;
}

static u32 space_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY(JOKER_EVENT_ON_HAND_PLAYED, joker_event)

    if (!joker_cards_one_in(rng_get_u32(RNG_SEQ_JOKER_SPACE), SPACE_JOKER_CHANCE))
        return JOKER_EFFECT_FLAG_NONE;

    enum HandType hand_type = get_hand_type();
    HandLevelValues before = hand_get_values(hand_type);
    hand_level_up(hand_type, 1);
    HandLevelValues after = hand_get_values(hand_type);

    // The played hand already has its chips and mult: add what the new level gives on top
    *joker_effect = &s_shared_joker_effect;
    (*joker_effect)->chips = after.chips - before.chips;
    (*joker_effect)->mult = after.mult - before.mult;
    return JOKER_EFFECT_FLAG_CHIPS | JOKER_EFFECT_FLAG_MULT;
}

static u32 hiker_joker_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    SCORE_ON_EVENT_ONLY_WITH_CARD(scored_card, JOKER_EVENT_ON_CARD_SCORED, joker_event)

    scored_card->bonus_chips = joker_cards_hiker_bonus(scored_card->bonus_chips);

    *joker_effect = &s_shared_joker_effect;
    (*joker_effect)->message = "+5 фиш.";
    return JOKER_EFFECT_FLAG_MESSAGE;
}

#pragma endregion
