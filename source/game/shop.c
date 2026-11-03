/**
 * @file shop.c
 *
 * @brief Shop state functions implementation.
 */

#include "game/shop.h"

#include "audio_utils.h"
#include "background_shop_gfx.h"
#include "bitset.h"
#include "booster_pack.h"
#include "button.h"
#include "card.h"
#include "consumable.h"
#include "game.h"
#include "game/blind_select.h"
#include "game/joker_row.h"
#include "game_variables.h"
#include "hand.h"
#include "joker.h"
#include "layout.h"
#include "list.h"
#include "mgba_logger.h"
#include "random.h"
#include "save.h"
#include "shop_economy.h"
#include "skip_tag.h"
#include "soundbank.h"
#include "state_machine.h"
#include "timer.h"
#include "util.h"
#include "voucher.h"

#include <string.h>

// Timer defs
#define TM_END_GAME_SHOP_INTRO    12
#define TM_CREATE_SHOP_ITEMS_WAIT 1
#define TM_SHIFT_SHOP_ICON_WAIT   7
#define TM_SHOW_ITEM_DESC_WAIT    12
#define TM_HIDE_DECK_WAIT         5

// Pixel sized
#define ITEM_SHOP_Y               71
#define OWNED_ITEMS_HIDE_Y_OFFSET 50

// Shop

// Weights of the top row item types in tenths, as in the original game (Jokers 20, Planets 4,
// Tarots 4, playing cards 0 or 4). The Planet and Tarot Vouchers change the consumables' weight.
#define SHOP_JOKER_WEIGHT_X10 200
#define NEXT_ROUND_BTN_SEL_X  0
#define REROLL_BTN_SEL_X      0
#define VOUCHER_SEL_X         1

#define SHOP_PACK_WEIGHT 8 // Booster Packs share the top row

// Opened Booster Pack
#define PACK_CHOICES_SPACING_PX 33
#define PACK_CHOICES_CENTER_X   154 // Middle of the green area right of the Shop icon
#define PACK_CHOICES_Y          10
#define PACK_TEXT_POS_X         76 // Title and card info, in the green area under the choices
#define PACK_TITLE_POS_Y        44
#define PACK_CARD_INFO_POS_Y    53
#define PACK_CARD_EDITION_DY    9 // The edition of a card goes on the line below its modifiers
#define PACK_HINT_POS_X         116 // Hint, in the Shop panel right of the Reroll button
#define PACK_HINT_POS_Y         72
// Top row of the Shop: cards are spread over its width when there are more than 2 of them
#define SHOP_TOP_ROW_CENTER_X 152
#define SHOP_TOP_ROW_WIDTH    76

// Palette IDs
#define REROLL_BTN_PAL_IDX                     3
#define NEXT_ROUND_BTN_SELECTED_BORDER_PAL_IDX 5
#define SHOP_PANEL_SHADOW_PAL_IDX              6
#define REROLL_BTN_SELECTED_BORDER_PAL_IDX     7
#define SHOP_LIGHTS_1_PAL_IDX                  8
#define SHOP_LIGHTS_2_PAL_IDX                  14
#define NEXT_ROUND_BTN_PAL_IDX                 16
#define SHOP_LIGHTS_3_PAL_IDX                  17
#define SHOP_LIGHTS_4_PAL_IDX                  22
#define SHOP_BOTTOM_PANEL_BORDER_PAL_IDX       26
#define SHOP_DESC_RARITY_MAIN_COLOR_PAL_IDX    27
#define SHOP_DESC_RARITY_SHADOW_COLOR_PAL_IDX  28

#define SHOP_LIGHTS_1_CLR 0xFFFF
#define SHOP_LIGHTS_2_CLR 0x32BE
#define SHOP_LIGHTS_3_CLR 0x4B5F
#define SHOP_LIGHTS_4_CLR 0x5F9F

// clang-format off
// Positions in tiles
static const Rect     SHOP_ICON_FROM_RECT           = {  0, 26,  8, 26};
static const BG_POINT SHOP_ICON_TO_POS              = {  0,  0};
static const BG_POINT OWNED_ITEMS_PANEL_3X3_SRC_POS = { 29, 21};
static const Rect     OWNED_JOKERS_PANEL_RECT       = {  9,  1, 21,  5};
static const Rect     OWNED_CONSUMABLES_PANEL_RECT  = { 23,  1, 28,  5};
static const Rect     OWNED_ITEMS_PANEL_RECT        = {  9,  1, 28,  5};
static const Rect     OWNED_ITEMS_PANEL_ANIM_CLEAR  = {  9,  0, 28,  1};
static const Rect     ITEM_DESC_9_PTCH_TO_RECT      = {  9,  6, 28, 18};
static const NinePatchRect ITEM_DESC_9_PTCH_SRC = {
                                        .patch_rect = { 27, 25, 31, 31},
                                        .margins    = {  2,  3,  2,  3}
};
static const int      ITEM_DESC_MAX_TEXT_HEIGHT     = ITEM_DESC_9_PTCH_TO_RECT.bottom -
                                                      ITEM_DESC_9_PTCH_TO_RECT.top + 1 -
                                                      ITEM_DESC_9_PTCH_SRC.margins.top -
                                                      ITEM_DESC_9_PTCH_SRC.margins.bottom;
static const Rect     ITEM_DESC_TEXT_RECT           = { 11,  9, 26, 18};
static const Rect     ITEM_NAME_TEXT_RECT           = { 10,  7, 27,  7};

// Positions in pixels
static const BG_POINT SHOP_JOKER_SPRITES_INIT_POS = {120, 160};
// The Voucher sits in the bottom right box of the Shop panel (the ticket is 28 pixels wide
// inside its 32 pixels sprite), its price on its right
static const BG_POINT SHOP_VOUCHER_POS            = {135, 113};
static const Rect     SHOP_VOUCHER_PRICE_RECT     = {169, 125, 192, 132 };
static const BG_POINT ITEM_DESCRIPTION_SPRITE_POS = {135,   9};
static const Rect     SHOP_PRICES_TEXT_RECT       = { 72,  56, 192, 160 };
static const Rect     PACK_TEXT_RECT              = { 72,  40, 240, 160 };
static const Rect     SHOP_REROLL_RECT            = { 88,  96, UNDEFINED, UNDEFINED };
// clang-format on

static List s_shop_items_list = LIST_DEFAULT;

// Voucher offered in the bottom left box, NULL if none
static Item* s_shop_voucher = NULL;

enum GameShopStates
{
    GAME_SHOP_INTRO,
    GAME_SHOP_SKIP_TAGS,
    GAME_SHOP_ACTIVE,
    GAME_SHOP_SHOW_ITEM_DESC,
    GAME_SHOP_HIDE_ITEM_DESC,
    GAME_SHOP_PACK_OPEN,
    GAME_SHOP_EXIT,
    GAME_SHOP_MAX
};

static void shop_intro(void);
static void shop_redeem_skip_tags(void);
static void shop_process_user_input(void);
static void shop_show_item_desc_on_init(void);
static void shop_show_item_desc_on_update(void);
static void shop_hide_item_desc_on_init(void);
static void shop_hide_item_desc_on_update(void);
static void shop_hide_item_desc_on_exit(void);
static void shop_pack_open_on_update(void);
static void shop_outro(void);

// clang-format off
static StateInfo shop_state_actions[GAME_SHOP_MAX] = {
    [GAME_SHOP_INTRO]          = STATE_INFO_UPDATE_FN_ONLY(shop_intro),
    [GAME_SHOP_SKIP_TAGS]      = STATE_INFO_UPDATE_FN_ONLY(shop_redeem_skip_tags),
    [GAME_SHOP_ACTIVE]         = STATE_INFO_UPDATE_FN_ONLY(shop_process_user_input),
    [GAME_SHOP_SHOW_ITEM_DESC] = STATE_INFO_INIT_UPDATE_FN(
                                    shop_show_item_desc_on_init,
                                    shop_show_item_desc_on_update),
    [GAME_SHOP_HIDE_ITEM_DESC] = {.on_init   = shop_hide_item_desc_on_init,
                                  .on_update = shop_hide_item_desc_on_update,
                                  .on_exit   = shop_hide_item_desc_on_exit},
    [GAME_SHOP_PACK_OPEN]      = STATE_INFO_UPDATE_FN_ONLY(shop_pack_open_on_update),
    [GAME_SHOP_EXIT]           = STATE_INFO_UPDATE_FN_ONLY(shop_outro),
};
// clang-format on

static StateMachine shop_sm = STATE_MACHINE_DEFINE(shop_state_actions, GAME_SHOP_MAX);

// Shop SelectionGrid

#define SHOP_GRID_OWNED_ROW    0
#define SHOP_GRID_FOR_SALE_ROW 1
#define SHOP_GRID_REROLL_ROW   2

static int shop_top_row_get_size(void);
static bool shop_top_row_on_selection_changed(
    SelectionGrid* selection_grid,
    int row_idx,
    const Selection* prev_selection,
    const Selection* new_selection
);
static void shop_top_row_on_key_transit(SelectionGrid* selection_grid, Selection* selection);
static int shop_reroll_row_get_size(void);
static bool shop_reroll_row_on_selection_changed(
    SelectionGrid* selection_grid,
    int row_idx,
    const Selection* prev_selection,
    const Selection* new_selection
);
static void shop_reroll_row_on_key_transit(SelectionGrid* selection_grid, Selection* selection);

static SelectionGridRow shop_selection_rows[] = {
    {0, jokers_sel_row_get_size,  jokers_sel_row_on_selection_changed,  jokers_sel_row_on_key_transit,  {.wrap = false, .has_h_exit_idx = false, .h_exit_idx = 0}},
    {1, shop_top_row_get_size,    shop_top_row_on_selection_changed,    shop_top_row_on_key_transit,    {.wrap = false, .has_h_exit_idx = false, .h_exit_idx = 0}},
    {2, shop_reroll_row_get_size, shop_reroll_row_on_selection_changed, shop_reroll_row_on_key_transit, {.wrap = false, .has_h_exit_idx = true, .h_exit_idx = 1} },
};

static const Selection SHOP_INIT_SEL = {-1, 1};

static SelectionGrid shop_selection_grid = {
    shop_selection_rows,
    NUM_ELEM_IN_ARR(shop_selection_rows),
    SHOP_INIT_SEL
};

// Shop Buttons

static void next_round_on_pressed(void);
static void reroll_on_pressed(void);
static bool reroll_can_be_pressed(void);
static Button next_round_button =
    {NEXT_ROUND_BTN_SELECTED_BORDER_PAL_IDX, NEXT_ROUND_BTN_PAL_IDX, next_round_on_pressed, NULL};
static Button reroll_button = {
    REROLL_BTN_SELECTED_BORDER_PAL_IDX,
    REROLL_BTN_PAL_IDX,
    reroll_on_pressed,
    reroll_can_be_pressed
};

// Shop internal variables

static int s_timer;

static int s_reroll_cost = SHOP_REROLL_BASE_COST;
static bool s_free_reroll_used = false; // Chaos the Clown: the free reroll of this Shop was used

/**
 * @brief What the next reroll costs: $0 with Chaos the Clown until its free reroll is used.
 */
static int shop_current_reroll_cost(void)
{
    return shop_economy_current_reroll_cost(
        s_reroll_cost,
        is_joker_owned(CHAOS_CLOWN_JOKER_ID),
        s_free_reroll_used
    );
}

/**
 * @brief Reroll cost at the start of each Shop, lowered by Reroll Surplus / Reroll Glut.
 */
static int shop_get_reroll_base_cost(void)
{
    return shop_economy_reroll_base_cost(voucher_get_reroll_discount());
}

// Variables relative to the Card we are showing the description of

static Item* s_description_item = NULL;
static FIXED s_description_item_original_x_pos = UNDEFINED;
static FIXED s_description_item_original_y_pos = UNDEFINED;
static int s_show_description_anim_progress = 0;

Item* shop_get_description_item(void)
{
    return s_description_item;
}

int shop_get_reroll_cost(void)
{
    return s_reroll_cost;
}

void shop_set_reroll_cost(int cost)
{
    s_reroll_cost = cost;
}

void shop_print_reroll_cost(void)
{
    tte_printf(
        "#{P:%d,%d; cx:0x%X000}$%d",
        SHOP_REROLL_RECT.left,
        SHOP_REROLL_RECT.top,
        TTE_WHITE_PB,
        shop_current_reroll_cost()
    );
}

void shop_reset(void)
{
    list_clear(&s_shop_items_list);
    s_shop_items_list = list_init();
    s_shop_voucher = NULL;
    s_reroll_cost = SHOP_REROLL_BASE_COST;
    s_free_reroll_used = false;
    joker_reset_rollable_jokers();
}

void shop_change_background(void)
{
    toggle_windows(false, true);

    GRIT_CPY(pal_bg_mem, background_shop_gfxPal);
    GRIT_CPY(&tile_mem[MAIN_BG_CBB], background_shop_gfxTiles);
    GRIT_CPY(&se_mem[MAIN_BG_SBB], background_shop_gfxMap);

    // Set the outline colors for the shop background. This is used for the alternate shop
    // palettes when opening packs
    pal_bg_mem[SHOP_BOTTOM_PANEL_BORDER_PAL_IDX] = 0x213D;
    pal_bg_mem[SHOP_PANEL_SHADOW_PAL_IDX] = 0x10B4;

    // Reset the shop lights to correct colors
    pal_bg_mem[SHOP_LIGHTS_2_PAL_IDX] = SHOP_LIGHTS_2_CLR;
    pal_bg_mem[SHOP_LIGHTS_3_PAL_IDX] = SHOP_LIGHTS_3_CLR;
    pal_bg_mem[SHOP_LIGHTS_4_PAL_IDX] = SHOP_LIGHTS_4_CLR;
    pal_bg_mem[SHOP_LIGHTS_1_PAL_IDX] = SHOP_LIGHTS_1_CLR;

    // Disable the button highlight colors
    pal_bg_mem[REROLL_BTN_SELECTED_BORDER_PAL_IDX] = pal_bg_mem[REROLL_BTN_PAL_IDX];
    pal_bg_mem[NEXT_ROUND_BTN_SELECTED_BORDER_PAL_IDX] = pal_bg_mem[NEXT_ROUND_BTN_PAL_IDX];
}

void shop_on_init(void)
{
    shop_change_background();

    s_timer = TM_ZERO;

    state_machine_register(&shop_sm);
    state_machine_change_state(&shop_sm, GAME_SHOP_INTRO);

    skip_tag_process_start(SKIP_TAG_EVENT_ON_SHOP_INIT);

    // The selection grid is initialized outside of bounds and moved
    // to trigger the selection change so the initial selection is visible
    shop_selection_grid.selection = SHOP_INIT_SEL;
    selection_grid_move_selection_horz(&shop_selection_grid, 1);
}

static void shop_redeem_skip_tags(void)
{
    if (skip_tag_process_get_proc_stage() == SKIP_TAG_PROCESS_STAGE_END)
    {
        s_timer = TM_ZERO;
        state_machine_change_state(&shop_sm, GAME_SHOP_ACTIVE);
    }
}

/**
 * @brief Create a shop top row item: a Joker or a Planet, possibly playing cards later.
 */
#ifdef DEBUG_GIVE_TAROT
// The emulator pokes 1 (Pack), 2 (playing card) or 3 (Planet or Tarot) here to force the item type
// of the top row (0: roll it normally)
volatile int g_debug_force_top_row = 0;
#endif

static Item* shop_create_top_row_item(void)
{
#ifdef DEBUG_GIVE_TAROT
    if (g_debug_force_top_row == 1)
        return item_roll_new(ITEM_TYPE_PACK, RNG_SEQ_SHOP_PACKS);
    if (g_debug_force_top_row == 2)
        return item_roll_new(ITEM_TYPE_PLAYING_CARD, RNG_SEQ_SHOP_PLAYING_CARDS);
    if (g_debug_force_top_row == 3)
        return item_roll_new(ITEM_TYPE_CONSUMABLE, RNG_SEQ_SHOP_PLANETS);
#endif

    // Smallest integer weights, so that the default 20:4 odds use the same modulus as before
    // and a given seed keeps rolling the same Shops
    // A slot of consumables (Planet or Tarot) weighs the mean of both: 4 without Vouchers
    int joker_weight = SHOP_JOKER_WEIGHT_X10;
    int planet_weight =
        (voucher_get_planet_shop_weight_x10() + voucher_get_tarot_shop_weight_x10()) / 2;
    int pack_weight = SHOP_PACK_WEIGHT * 10;
    int card_weight = voucher_get_playing_card_shop_weight_x10(); // Magic Trick
    int divisor = 10;
    if (planet_weight % 10 != 0 || card_weight % 10 != 0)
        divisor = 2;
    joker_weight /= divisor;
    planet_weight /= divisor;
    pack_weight /= divisor;
    card_weight /= divisor;

    u32 roll = rng_get_u32(RNG_SEQ_SHOP_ITEM_TYPE) %
               (joker_weight + planet_weight + pack_weight + card_weight);

    if (roll >= joker_weight + planet_weight + pack_weight)
    {
        // Magic Trick: a playing card. NULL when no sprite layer is free: fall back to a Joker
        Item* card = item_roll_new(ITEM_TYPE_PLAYING_CARD, RNG_SEQ_SHOP_PLAYING_CARDS);
        if (card != NULL)
            return card;
        roll = 0;
    }
    else if (roll >= joker_weight + planet_weight)
    {
        // NULL when no pack sprite is free: fall back to a Joker
        Item* pack = item_roll_new(ITEM_TYPE_PACK, RNG_SEQ_SHOP_PACKS);
        if (pack != NULL)
            return pack;
        roll = 0;
    }

    if (roll < joker_weight)
    {
        Item* joker = item_roll_new(ITEM_TYPE_JOKER, RNG_SEQ_SHOP_ITEMS);
        // No Joker left to roll: fall back to a Planet
        if (joker != NULL)
            return joker;
    }

    return item_roll_new(ITEM_TYPE_CONSUMABLE, RNG_SEQ_SHOP_PLANETS);
}

/**
 * @brief Horizontal position of a card in the Shop's top row.
 *
 * @param idx index of the card in the row
 * @param nb_items number of cards in the row: up to 2 side by side, more are spread over the row
 */
static int shop_top_row_item_x(int idx, int nb_items)
{
    int spacing = CARD_SPRITE_SIZE_PX;
    if (nb_items > 2)
        spacing = (SHOP_TOP_ROW_WIDTH - CARD_SPRITE_SIZE_PX) / (nb_items - 1);

    int first_x = SHOP_TOP_ROW_CENTER_X - ((nb_items - 1) * spacing) / 2 - CARD_SPRITE_SIZE_PX / 2;
    return first_x + idx * spacing;
}

/**
 * @brief Setup for the lists of items we can purchase in the top row of the Shop.
 *        i.e. Jokers and consumables and possibly playing cards.
 */
static void shop_create_top_row_items(void)
{
    tte_erase_rect_wrapper(SHOP_PRICES_TEXT_RECT);

    List* shop_items_list = &s_shop_items_list;

    list_clear(shop_items_list);
    *shop_items_list = list_init();

    int shop_slots = g_game_vars.shop_slots;
    if (shop_slots > MAX_SHOP_ITEMS)
        shop_slots = MAX_SHOP_ITEMS;

    for (int i = 0; i < shop_slots; i++)
    {
        Item* item = shop_create_top_row_item();

        if (item == NULL)
        {
            MGBA_FUNC_WARN("Could not create shop item");

            // TODO: Decide how to handle this case gracefully
            // If the issue for example is that we can't generate any more jokers,
            // because for example the user owns all of them,
            // maybe we need to generate consumables instead.
            return;
        }

        item->x = int2fx(shop_top_row_item_x(i, shop_slots));
        item->y = int2fx(SHOP_JOKER_SPRITES_INIT_POS.y);
        item->tx = item->x;
        item->ty = int2fx(ITEM_SHOP_Y);

        item_print_buy_price_under(item);

        list_push_back(shop_items_list, item);
    }
}

static void shop_print_voucher_price(void)
{
    tte_erase_rect_wrapper(SHOP_VOUCHER_PRICE_RECT);
    if (s_shop_voucher == NULL)
        return;

    tte_printf(
        "#{P:%d,%d; cx:0x%X000}$%d",
        SHOP_VOUCHER_PRICE_RECT.left,
        SHOP_VOUCHER_PRICE_RECT.top,
        TTE_YELLOW_PB,
        item_get_buy_price(s_shop_voucher)
    );
}

/**
 * @brief Reprints every price of the Shop: top row cards, reroll and Voucher.
 */
static void shop_print_all_prices(void)
{
    tte_erase_rect_wrapper(SHOP_PRICES_TEXT_RECT);

    Item* item;
    ListItr itr = list_itr_create(&s_shop_items_list);
    while ((item = list_itr_next(&itr)))
    {
        item_print_buy_price_under(item);
    }

    shop_print_reroll_cost();
    shop_print_voucher_price();
}

/**
 * @brief Puts the Voucher of the current Ante in the bottom left box, if it wasn't bought yet.
 */
static void shop_create_voucher(void)
{
    s_shop_voucher = NULL;

    enum VoucherType voucher = voucher_get_shop_offer();
    if (voucher == VOUCHER_NONE)
        return;

    s_shop_voucher = (Item*)voucher_object_new(voucher);
    if (s_shop_voucher == NULL)
        return;

    s_shop_voucher->x = int2fx(SHOP_VOUCHER_POS.x);
    s_shop_voucher->y = int2fx(SHOP_JOKER_SPRITES_INIT_POS.y);
    s_shop_voucher->tx = s_shop_voucher->x;
    s_shop_voucher->ty = int2fx(SHOP_VOUCHER_POS.y);

    shop_print_voucher_price();
}

/**
 * @brief Adds one card to the top row when Overstock is redeemed in the Shop, and spreads the
 *        row again.
 */
static void shop_add_top_row_item(void)
{
    Item* new_item = shop_create_top_row_item();
    if (new_item == NULL)
        return;

    new_item->y = int2fx(SHOP_JOKER_SPRITES_INIT_POS.y);
    new_item->ty = int2fx(ITEM_SHOP_Y);
    list_push_back(&s_shop_items_list, new_item);

    int nb_items = list_get_len(&s_shop_items_list);
    int idx = 0;
    Item* item;
    ListItr itr = list_itr_create(&s_shop_items_list);
    while ((item = list_itr_next(&itr)))
    {
        // Moved right away so that the prices are printed under the new positions
        item->tx = int2fx(shop_top_row_item_x(idx, nb_items));
        item->x = item->tx;
        idx++;
    }

    shop_print_all_prices();
}

/**
 * @brief Intro sequence (menu and shop icon coming into frame)
 */
static void shop_intro()
{
    main_bg_se_copy_rect_1_tile_vert(POP_MENU_ANIM_RECT, SCREEN_UP);

    if (s_timer == TM_CREATE_SHOP_ITEMS_WAIT)
    {
        shop_create_top_row_items();
        shop_create_voucher();
    }

    if (s_timer >= TM_SHIFT_SHOP_ICON_WAIT) // Shift the shop icon
    {
        int timer_offset = s_timer - 6;

        // TODO: Extract to generic function?
        for (int y = 0; y < timer_offset; y++)
        {
            Rect from = SHOP_ICON_FROM_RECT;
            from.top += y - timer_offset;
            from.bottom += y - timer_offset;

            BG_POINT to = SHOP_ICON_TO_POS;
            to.y = y;

            main_bg_se_copy_rect(from, to);
        }
    }

    if (s_timer == TM_END_GAME_SHOP_INTRO)
    {
        state_machine_change_state(&shop_sm, GAME_SHOP_SKIP_TAGS);
        s_timer = TM_ZERO; // Reset the timer

        // print initial reroll cost only when the panel is in place
        shop_print_reroll_cost();
    }
}

/**
 * @brief Computes the number of Jokers up for sale, aka the number of
 *         "buttons" there are on the top row of the Shop.
 */
static int shop_top_row_get_size(void)
{
    // + 1 to account for next round button
    return list_get_len(&s_shop_items_list) + 1;
}

/**
 * @brief Called when pressing A on a Shop Joker to buy it.
 */
static BoosterPackObject* s_opened_pack = NULL;
static bool s_pack_open_requested = false;

static inline void shop_buy_item(int shop_item_idx)
{
    List* shop_items_list = &s_shop_items_list;
    Item* item = (Item*)list_get_at_idx(shop_items_list, shop_item_idx);

    g_game_vars.money -= item_get_buy_price(item);
    display_money();
    sprite_object_erase_text_under((SpriteObject*)item);
    sprite_object_set_focus((SpriteObject*)item, false);
    list_remove_at_idx(shop_items_list, shop_item_idx); // Remove the item from the shop

    if (item->type == ITEM_TYPE_PACK)
    {
        // Packs are opened right away instead of being acquired
        s_opened_pack = (BoosterPackObject*)item;
        s_pack_open_requested = true;
        return;
    }

    item_acquire(item);
}

static void shop_pack_open_start(void);

/**
 * @brief Handle button inputs for the "Next Round" button and shop Jokers.
 */
static void shop_top_row_on_key_transit(SelectionGrid* selection_grid, Selection* selection)
{
    if (!key_hit(KEY_CONFIRM))
        return;

    if (selection->x == NEXT_ROUND_BTN_SEL_X)
    {
        button_press(&next_round_button);
    }
    else
    {
        int shop_item_idx = selection->x - 1; // - 1 to account for next round button
        Item* item = (Item*)list_get_at_idx(&s_shop_items_list, shop_item_idx);
        if (!item_can_acquire(item) || !game_can_afford(item_get_buy_price(item)))
        {
            return;
        }

        shop_buy_item(shop_item_idx);
        selection_grid_move_selection_horz(selection_grid, -1);

        if (s_pack_open_requested)
        {
            s_pack_open_requested = false;
            shop_pack_open_start();
        }
    }
}

/**
 * @brief Handle d-pad inputs for the "Next Round" button and shop Jokers' row.
 */
static bool shop_top_row_on_selection_changed(
    SelectionGrid* selection_grid,
    int row_idx,
    const Selection* prev_selection,
    const Selection* new_selection
)
{
    List* shop_items_list = &s_shop_items_list;
    // Guard if we move down while on jokers
    if (new_selection->y > row_idx && prev_selection->x > 0)
        return false;

    // The selection grid system only guarantees that the new selection is within bounds
    // but not the previous one...
    // This allows using INIT_SEL = {-1, 1} and move to set the initial selection in a hacky way...
    if (prev_selection->y == row_idx && prev_selection->x >= 0 &&
        prev_selection->x < shop_top_row_get_size())
    {
        if (prev_selection->x == NEXT_ROUND_BTN_SEL_X)
        {
            button_set_highlight(&next_round_button, false);
        }
        else
        {
            int idx = prev_selection->x - 1; // -1 to account for next round button
            SpriteObject* sprite_object = (SpriteObject*)list_get_at_idx(shop_items_list, idx);
            sprite_object_set_focus(sprite_object, false);
        }
    }

    if (new_selection->y == row_idx)
    {
        if (new_selection->x == NEXT_ROUND_BTN_SEL_X)
        {
            button_set_highlight(&next_round_button, true);
        }
        else
        {
            int idx = new_selection->x - 1; // -1 to account for next round button
            SpriteObject* sprite_object = (SpriteObject*)list_get_at_idx(shop_items_list, idx);
            sprite_object_set_focus(sprite_object, true);
        }
    }

    return true;
}

/**
 * @brief Get size of the row with the "Reroll" button
 *
 * @returns 2 when a Voucher is offered (Reroll button, then the Voucher), 1 otherwise.
 */
static int shop_reroll_row_get_size()
{
    // Without a Voucher, left/right on the Reroll button goes to the top row like before
    shop_selection_rows[SHOP_GRID_REROLL_ROW].attributes.has_h_exit_idx = (s_shop_voucher == NULL);
    return (s_shop_voucher != NULL) ? 2 : 1;
}

/**
 * @brief Handle d-pad inputs for the "Reroll" button's row.
 */
static bool shop_reroll_row_on_selection_changed(
    SelectionGrid* selection_grid,
    int row_idx,
    const Selection* prev_selection,
    const Selection* new_selection
)
{
    if (row_idx == prev_selection->y)
    {
        if (prev_selection->x == VOUCHER_SEL_X)
            sprite_object_set_focus((SpriteObject*)s_shop_voucher, false);
        else
            button_set_highlight(&reroll_button, false);

        // Leaving through the right of the Reroll button goes to the top row's first card
        if (new_selection->y == SHOP_GRID_FOR_SALE_ROW && new_selection->x != NEXT_ROUND_BTN_SEL_X)
        {
            int idx = new_selection->x - 1;
            SpriteObject* sprite_object = (SpriteObject*)list_get_at_idx(&s_shop_items_list, idx);
            sprite_object_set_focus(sprite_object, true);
        }
    }

    if (row_idx == new_selection->y)
    {
        if (new_selection->x == VOUCHER_SEL_X)
            sprite_object_set_focus((SpriteObject*)s_shop_voucher, true);
        else
            button_set_highlight(&reroll_button, true);
    }

    return true;
}

/**
 * @brief Applies what a redeemed Voucher changes in the Shop that is open.
 */
static void shop_on_voucher_redeemed(enum VoucherType voucher)
{
    switch (voucher)
    {
        case VOUCHER_OVERSTOCK:
        case VOUCHER_OVERSTOCK_PLUS:
            if (list_get_len(&s_shop_items_list) < MAX_SHOP_ITEMS)
                shop_add_top_row_item();
            break;

        case VOUCHER_REROLL_SURPLUS:
        case VOUCHER_REROLL_GLUT:
            s_reroll_cost = shop_economy_reroll_cost_after_voucher(s_reroll_cost);
            break;

        default:
            break;
    }

    // Discounts change the prices, the reroll cost may have changed
    shop_print_all_prices();
}

/**
 * @brief Buys the Voucher offered in the Shop.
 */
static void shop_buy_voucher(SelectionGrid* selection_grid)
{
    Item* voucher = s_shop_voucher;
    if (voucher == NULL || !item_can_acquire(voucher) ||
        !game_can_afford(item_get_buy_price(voucher)))
    {
        return;
    }

    // Move the selection back to the Reroll button while the Voucher still exists
    selection_grid_move_selection_horz(selection_grid, -1);

    enum VoucherType voucher_type = ((VoucherObject*)voucher)->voucher_type;
    g_game_vars.money -= item_get_buy_price(voucher);
    display_money();
    sprite_object_set_focus((SpriteObject*)voucher, false);
    s_shop_voucher = NULL;
    item_acquire(voucher); // Redeems it and starts its discard animation

    shop_on_voucher_redeemed(voucher_type);
}

/**
 * @brief Reroll items up for sale in the Shop.
 *         Reroll cost will go up by a rate that increases by 1 each reroll.
 */
static inline void shop_reroll(void)
{
    joker_on_shop_reroll(); // Flash Card
    int paid_cost = shop_current_reroll_cost();
    g_game_vars.money -= paid_cost;
    display_money(); // Update the money display

    List* shop_items_list = &s_shop_items_list;
    ListItr itr = list_itr_create(shop_items_list);
    Item* item;

    while ((item = list_itr_next(&itr)))
    {
        if (item != NULL)
        {
            item_dispose(&item);
        }
    }

    list_clear(shop_items_list);
    *shop_items_list = list_init();

    shop_create_top_row_items();

    itr = list_itr_create(shop_items_list);

    SpriteObject* item_sprite_object;
    while ((item_sprite_object = list_itr_next(&itr)))
    {
        if (item_sprite_object != NULL)
        {
            item_sprite_object->y = item_sprite_object->ty;

            sprite_object_shake(item_sprite_object, UNDEFINED);
        }
    }

    // Chaos the Clown: the free reroll doesn't raise the price
    s_reroll_cost =
        shop_economy_reroll_cost_after_reroll(s_reroll_cost, paid_cost, &s_free_reroll_used);
    shop_print_reroll_cost();
    // The top row prices were erased with the Voucher's
    shop_print_voucher_price();
}

/**
 * @brief Handle button inputs for the "Reroll" button's row.
 */
static void shop_reroll_row_on_key_transit(SelectionGrid* selection_grid, Selection* selection)
{
    if (!key_hit(KEY_CONFIRM))
    {
        return;
    }

    if (selection->x == VOUCHER_SEL_X)
        shop_buy_voucher(selection_grid);
    else
        button_press(&reroll_button);
}

static void next_round_on_pressed(void)
{
    joker_on_shop_exit(); // Perkeo

    // Go to next blind selection game state
    state_machine_change_state(&shop_sm, GAME_SHOP_EXIT);
    s_timer = TM_ZERO;
    s_reroll_cost = shop_get_reroll_base_cost();
    s_free_reroll_used = false;

    pal_bg_mem[NEXT_ROUND_BTN_SELECTED_BORDER_PAL_IDX] = pal_bg_mem[SHOP_PANEL_SHADOW_PAL_IDX];
}

static void reroll_on_pressed(void)
{
    // TODO: Add money sound effect
    shop_reroll();
}

static bool reroll_can_be_pressed(void)
{
    return game_can_afford(shop_current_reroll_cost());
}

/**
 * @brief Handle user inputs logic in the Shop though a SelectionGrid.
 */
static void shop_process_user_input(void)
{
    selection_grid_process_input(&shop_selection_grid);

    static Item* new_description_item = NULL;

    // Determine the Item we would show the description of
    switch (shop_selection_grid.selection.y)
    {
        case SHOP_GRID_OWNED_ROW:
        {
            new_description_item = owned_items_row_get_item(shop_selection_grid.selection.x);
            break;
        }

        case SHOP_GRID_FOR_SALE_ROW:
        {
            // Ignore the "Next Round" button when selecting descripted Item: get it at
            // selection.x - 1 in the list to account for this next round button being ignored.
            new_description_item =
                (shop_selection_grid.selection.x != NEXT_ROUND_BTN_SEL_X)
                    ? list_get_at_idx(&s_shop_items_list, shop_selection_grid.selection.x - 1)
                    : NULL;
            break;
        }

        case SHOP_GRID_REROLL_ROW:
        {
            new_description_item =
                (shop_selection_grid.selection.x == VOUCHER_SEL_X) ? s_shop_voucher : NULL;
            break;
        }

        default:
        {
            new_description_item = NULL;
            break;
        }
    }

    // Show description of selected card when pressing B
    if (new_description_item != NULL && key_held(KEY_SHOW_DESCRIPTION))
    {
        s_description_item = new_description_item;
        s_description_item_original_x_pos = s_description_item->tx;
        s_description_item_original_y_pos = s_description_item->ty;

        s_timer = TM_ZERO;
        state_machine_change_state(&shop_sm, GAME_SHOP_SHOW_ITEM_DESC);
    }
}

static void shop_show_item_desc_on_init(void)
{
    // This starts at 0, then gets incremented up to TM_SHOW_ITEM_DESC_WAIT. Will be used to
    // revert the animation if the B button is released midway through it
    s_show_description_anim_progress = 0;

    // Erase shop/round text
    tte_erase_rect_wrapper(PLAYING_SCREEN_RECT);

    // Move all other Sprites offscreen

    Item* tmp_item = NULL;

    // Owned Jokers and consumables
    ListItr itr = list_itr_create(get_jokers_list());
    while ((tmp_item = (Item*)list_itr_next(&itr)))
    {
        if (tmp_item != s_description_item)
            tmp_item->ty -= int2fx(OWNED_ITEMS_HIDE_Y_OFFSET);
    }
    itr = list_itr_create(get_consumables_list());
    while ((tmp_item = (Item*)list_itr_next(&itr)))
    {
        if (tmp_item != s_description_item)
            tmp_item->ty -= int2fx(OWNED_ITEMS_HIDE_Y_OFFSET);
    }

    // Shop Jokers
    itr = list_itr_create(&s_shop_items_list);
    while ((tmp_item = (Item*)list_itr_next(&itr)))
    {
        if (tmp_item != s_description_item)
            tmp_item->ty = int2fx(SHOP_JOKER_SPRITES_INIT_POS.y + TILE_SIZE);
    }

    // Shop Voucher
    if (s_shop_voucher != NULL && s_shop_voucher != s_description_item)
        s_shop_voucher->ty = int2fx(SHOP_JOKER_SPRITES_INIT_POS.y + TILE_SIZE);

    set_owned_skip_tags_moved_offscreen(true);

    // Set description_card new target position

    s_description_item->tx = int2fx(ITEM_DESCRIPTION_SPRITE_POS.x);
    s_description_item->ty = int2fx(ITEM_DESCRIPTION_SPRITE_POS.y);
}

static void shop_show_item_desc_on_update(void)
{
    if (s_timer <= TM_SHOW_ITEM_DESC_WAIT)
    {
        s_show_description_anim_progress++;

        // Hide Deck (last frames only)
        if (TM_SHOW_ITEM_DESC_WAIT - s_timer < TM_HIDE_DECK_WAIT)
            main_bg_se_move_rect_1_tile_vert(DECK_ANIM_RECT, SCREEN_DOWN);
        // Hide shop panel
        main_bg_se_move_rect_1_tile_vert(POP_MENU_ANIM_RECT, SCREEN_DOWN);
        // Hide Owned Cards panels
        main_bg_se_move_rect_1_tile_vert(OWNED_ITEMS_PANEL_RECT, SCREEN_UP);
    }

    // Anim end
    else if (s_timer == TM_SHOW_ITEM_DESC_WAIT + 1)
    {
        // Print the Item's name
        const char* item_name = item_get_name(s_description_item);
        tte_printf(
            TTE_WHITE_TAG "#{P:%d,%d}%*s%s",
            ITEM_NAME_TEXT_RECT.left * TILE_SIZE,
            ITEM_NAME_TEXT_RECT.top * TILE_SIZE,
            (rect_width(&ITEM_NAME_TEXT_RECT) - strlen(item_name)) / 2,
            "",
            item_name
        );

        // Compute needed space for the description and print it
        int nb_printed_lines = item_print_description(s_description_item, ITEM_DESC_TEXT_RECT);
        int desc_bottom_offset = ITEM_DESC_MAX_TEXT_HEIGHT - nb_printed_lines;

        // Print Rarity/Type and change the panel's color before drawing it so the color is already
        // set, in case there is any lag
        ItemDescSubtypeInfo item_subtype_info = item_get_subtype_info(s_description_item);
        const char* subtype_str = item_subtype_info.name_str;
        tte_printf(
            TTE_WHITE_TAG "#{P:%d,%d}%*s%s",
            ITEM_DESC_TEXT_RECT.left * TILE_SIZE,
            (ITEM_DESC_TEXT_RECT.bottom - desc_bottom_offset - 1) * TILE_SIZE,
            (rect_width(&ITEM_DESC_TEXT_RECT) - strlen(subtype_str)) / 2,
            "",
            subtype_str
        );

        pal_bg_mem[SHOP_DESC_RARITY_MAIN_COLOR_PAL_IDX] = item_subtype_info.main_color;
        pal_bg_mem[SHOP_DESC_RARITY_SHADOW_COLOR_PAL_IDX] = item_subtype_info.shadow_color;

        // Draw description panel
        Rect actual_dest_rect = ITEM_DESC_9_PTCH_TO_RECT;
        actual_dest_rect.bottom -= desc_bottom_offset;
        main_bg_se_copy_expand_9_patch(actual_dest_rect, &ITEM_DESC_9_PTCH_SRC);
    }

    // Actively wait for the B button to be released
    if (!key_held(KEY_SHOW_DESCRIPTION))
    {
        s_timer = TM_ZERO;
        state_machine_change_state(&shop_sm, GAME_SHOP_HIDE_ITEM_DESC);
    }
}

static void shop_hide_item_desc_on_init(void)
{
    // Erase shop text and Joker Description frame if we had time to draw them
    if (s_show_description_anim_progress >= TM_SHOW_ITEM_DESC_WAIT)
    {
        main_bg_se_clear_rect(ITEM_DESC_9_PTCH_TO_RECT);
    }
    // Or clear the owned cards' panel that haven't finished moving up
    else
    {
        main_bg_se_clear_rect(OWNED_ITEMS_PANEL_ANIM_CLEAR);
    }

    tte_erase_rect_wrapper(PLAYING_SCREEN_RECT);

    // Redraw Jokers/Consumables frames
    main_bg_se_copy_expand_3x3_rect(OWNED_JOKERS_PANEL_RECT, OWNED_ITEMS_PANEL_3X3_SRC_POS);
    main_bg_se_copy_expand_3x3_rect(OWNED_CONSUMABLES_PANEL_RECT, OWNED_ITEMS_PANEL_3X3_SRC_POS);

    // Move Sprites back to their positions

    Item* tmp_item = NULL;

    // Owned Items
    ListItr itr = list_itr_create(get_jokers_list());
    while ((tmp_item = (Item*)list_itr_next(&itr)))
    {
        if (tmp_item != s_description_item)
            tmp_item->ty = int2fx(HELD_JOKERS_POS.y);
    }
    itr = list_itr_create(get_consumables_list());
    while ((tmp_item = (Item*)list_itr_next(&itr)))
    {
        if (tmp_item != s_description_item)
            tmp_item->ty = int2fx(HELD_JOKERS_POS.y);
    }

    // Shop Jokers
    itr = list_itr_create(&s_shop_items_list);
    while ((tmp_item = (Item*)list_itr_next(&itr)))
    {
        if (tmp_item != s_description_item)
            tmp_item->ty = int2fx(ITEM_SHOP_Y);
    }

    // Shop Voucher
    if (s_shop_voucher != NULL && s_shop_voucher != s_description_item)
        s_shop_voucher->ty = int2fx(SHOP_VOUCHER_POS.y);

    set_owned_skip_tags_moved_offscreen(false);

    s_description_item->tx = s_description_item_original_x_pos;
    s_description_item->ty = s_description_item_original_y_pos;
}

static void shop_hide_item_desc_on_update(void)
{
    if (s_timer <= s_show_description_anim_progress)
    {
        // Show Deck (last frames only)
        if (s_show_description_anim_progress > TM_HIDE_DECK_WAIT &&
            s_timer < (s_show_description_anim_progress - (TM_HIDE_DECK_WAIT + 1)))
        {
            main_bg_se_move_rect_1_tile_vert(DECK_ANIM_RECT, SCREEN_UP);
        }
        // Show menu panel
        main_bg_se_move_rect_1_tile_vert(POP_MENU_ANIM_RECT, SCREEN_UP);
    }

    // Last anim frame (no need to wait for the Joker to have stopped for this):
    else if (s_timer == s_show_description_anim_progress + 1)
    {
        s_timer = TM_ZERO;
        state_machine_change_state(&shop_sm, GAME_SHOP_ACTIVE);
    }
}

static void shop_hide_item_desc_on_exit(void)
{
    // TODO: Do the following in a less hacky way

    // Need to account for the description_card being selected if it came from the shop.
    // Temporarily lower the target position of the Item so that the price is printed in the
    // appropriate place
    if (!item_is_owned(s_description_item))
        s_description_item->ty += int2fx(TILE_SIZE);

    // Print price under shop Jokers
    Item* item = NULL;
    ListItr itr = list_itr_create(&s_shop_items_list);
    while ((item = list_itr_next(&itr)))
    {
        item_print_buy_price_under(item);
    }

    // Revert what we just did so the Item still appears selected
    if (!item_is_owned(s_description_item))
        s_description_item->ty -= int2fx(TILE_SIZE);

    shop_print_reroll_cost();
    shop_print_voucher_price();

    // Print Deck size that was erased
    display_deck_size_max();

    // if we are NOT pressing A, print the price under the description card if it's a card we own.
    if (!key_held(KEY_CONFIRM) && item_is_owned(s_description_item))
    {
        sprite_object_print_price_under(
            (SpriteObject*)s_description_item,
            item_get_sell_price(s_description_item)
        );
    }

    s_description_item = NULL;
}

#pragma region Booster Pack opening

static const PackInfo* s_pack_info = NULL;
static Item* s_pack_choices[MAX_PACK_CHOICES];
static int s_pack_nb_choices = 0;
static int s_pack_selected = 0;
static int s_pack_picks_left = 0;

static void shop_pack_print_texts(void)
{
    tte_erase_rect_wrapper(PACK_TEXT_RECT);

    tte_printf(TTE_WHITE_TAG "#{P:%d,%d}%s", PACK_TEXT_POS_X, PACK_TITLE_POS_Y, s_pack_info->name);

    // Enhancement and seal of the selected card
    Item* choice = s_pack_choices[s_pack_selected];
    if (choice != NULL && choice->type == ITEM_TYPE_PLAYING_CARD)
    {
        const Card* card = ((CardObject*)choice)->card;
        const char* enhancement = card_get_enhancement_name(card);
        const char* seal = card_get_seal_name(card);
        tte_printf(
            TTE_YELLOW_TAG "#{P:%d,%d}%s%s%s",
            PACK_TEXT_POS_X,
            PACK_CARD_INFO_POS_Y,
            enhancement != NULL ? enhancement : "",
            enhancement != NULL && seal != NULL ? ", " : "",
            seal != NULL ? seal : ""
        );

        const char* edition = card_get_edition_name(card);
        if (edition != NULL)
        {
            tte_printf(
                TTE_BLUE_TAG "#{P:%d,%d}%s",
                PACK_TEXT_POS_X,
                PACK_CARD_INFO_POS_Y + PACK_CARD_EDITION_DY,
                edition
            );
        }
    }

    else if (choice != NULL && choice->type == ITEM_TYPE_CONSUMABLE)
    {
        tte_printf(
            TTE_YELLOW_TAG "#{P:%d,%d}%s",
            PACK_TEXT_POS_X,
            PACK_CARD_INFO_POS_Y,
            item_get_name(choice)
        );
    }

    tte_printf(TTE_WHITE_TAG "#{P:%d,%d}A: взять", PACK_HINT_POS_X, PACK_HINT_POS_Y);
    tte_printf(TTE_WHITE_TAG "#{P:%d,%d}START: пропуск", PACK_HINT_POS_X, PACK_HINT_POS_Y + 10);
    display_deck_size_max();
}

static void shop_pack_set_selected(int idx)
{
    if (s_pack_choices[s_pack_selected] != NULL)
        sprite_object_set_focus((SpriteObject*)s_pack_choices[s_pack_selected], false);
    s_pack_selected = idx;
    if (s_pack_choices[s_pack_selected] != NULL)
        sprite_object_set_focus((SpriteObject*)s_pack_choices[s_pack_selected], true);
    shop_pack_print_texts();
}

static void shop_pack_open_start(void)
{
    s_pack_info = booster_pack_get_info(s_opened_pack->pack_type);
    booster_pack_destroy(&s_opened_pack);
    joker_on_pack_opened(); // Hallucination

    // Move the other shop items out of the way
    ListItr itr = list_itr_create(&s_shop_items_list);
    Item* item;
    while ((item = list_itr_next(&itr)))
    {
        sprite_object_set_focus((SpriteObject*)item, false);
        item->ty = int2fx(SHOP_JOKER_SPRITES_INIT_POS.y + TILE_SIZE);
    }

    s_pack_nb_choices = 0;
    int nb_choices = s_pack_info->nb_choices;
    if (nb_choices > MAX_PACK_CHOICES)
        nb_choices = MAX_PACK_CHOICES;

    int start_x = PACK_CHOICES_CENTER_X - (nb_choices * PACK_CHOICES_SPACING_PX - 1) / 2;
    for (int i = 0; i < nb_choices; i++)
    {
        Item* choice = s_pack_info->roll_choice(i);
        s_pack_choices[i] = choice;
        if (choice == NULL)
            continue;
        choice->x = int2fx(start_x + i * PACK_CHOICES_SPACING_PX);
        choice->y = int2fx(SHOP_JOKER_SPRITES_INIT_POS.y);
        choice->tx = choice->x;
        choice->ty = int2fx(PACK_CHOICES_Y);
        s_pack_nb_choices++;
    }
    for (int i = nb_choices; i < MAX_PACK_CHOICES; i++)
        s_pack_choices[i] = NULL;

    s_pack_picks_left = s_pack_info->nb_picks;
    s_pack_selected = 0;
    while (s_pack_selected < nb_choices - 1 && s_pack_choices[s_pack_selected] == NULL)
        s_pack_selected++;

    s_timer = TM_ZERO;
    state_machine_change_state(&shop_sm, GAME_SHOP_PACK_OPEN);
    shop_pack_set_selected(s_pack_selected);
}

static void shop_pack_close(void)
{
    for (int i = 0; i < MAX_PACK_CHOICES; i++)
    {
        if (s_pack_choices[i] != NULL)
        {
            sprite_object_set_focus((SpriteObject*)s_pack_choices[i], false);
            s_pack_info->discard(&s_pack_choices[i]);
        }
    }
    s_pack_nb_choices = 0;
    s_pack_info = NULL;

    tte_erase_rect_wrapper(PACK_TEXT_RECT);

    // Bring the shop items back
    ListItr itr = list_itr_create(&s_shop_items_list);
    Item* item;
    while ((item = list_itr_next(&itr)))
    {
        item->ty = int2fx(ITEM_SHOP_Y);
        item_print_buy_price_under(item);
    }

    // Re-highlight what the shop selection points to
    int sel_x = shop_selection_grid.selection.x;
    if (shop_selection_grid.selection.y == SHOP_GRID_FOR_SALE_ROW && sel_x > NEXT_ROUND_BTN_SEL_X &&
        sel_x - 1 < list_get_len(&s_shop_items_list))
    {
        sprite_object_set_focus(
            (SpriteObject*)list_get_at_idx(&s_shop_items_list, sel_x - 1),
            true
        );
    }

    shop_print_reroll_cost();
    shop_print_voucher_price();
    display_deck_size_max();
    state_machine_change_state(&shop_sm, GAME_SHOP_ACTIVE);
}

static void shop_pack_open_on_update(void)
{
    if (s_timer < 2)
        return;

    if (key_hit(KEY_START))
    {
        joker_on_pack_skipped(); // Red Card
        shop_pack_close();
        return;
    }

    int dir = 0;
    if (key_hit(KEY_LEFT))
        dir = -1;
    else if (key_hit(KEY_RIGHT))
        dir = 1;

    if (dir != 0)
    {
        int idx = s_pack_selected;
        do
        {
            idx += dir;
        } while (idx >= 0 && idx < MAX_PACK_CHOICES && s_pack_choices[idx] == NULL);

        if (idx >= 0 && idx < MAX_PACK_CHOICES)
            shop_pack_set_selected(idx);
    }
    else if (key_hit(KEY_CONFIRM) && s_pack_choices[s_pack_selected] != NULL)
    {
        // E.g. no free consumable slot: the choice stays in the pack
        if (s_pack_info->can_pick != NULL && !s_pack_info->can_pick(s_pack_choices[s_pack_selected]))
        {
            play_sfx(SFX_CARD_DESELECT, MM_BASE_PITCH_RATE, SFX_DEFAULT_VOLUME);
            return;
        }

        sprite_object_set_focus((SpriteObject*)s_pack_choices[s_pack_selected], false);
        s_pack_info->pick(&s_pack_choices[s_pack_selected]);
        s_pack_nb_choices--;
        s_pack_picks_left--;

        if (s_pack_picks_left <= 0 || s_pack_nb_choices <= 0)
        {
            shop_pack_close();
            return;
        }

        // Move to the closest remaining choice
        for (int i = 0; i < MAX_PACK_CHOICES; i++)
        {
            if (s_pack_choices[i] != NULL)
            {
                s_pack_selected = i;
                break;
            }
        }
        sprite_object_set_focus((SpriteObject*)s_pack_choices[s_pack_selected], true);
        shop_pack_print_texts();
    }
}

#pragma endregion

/**
 * @brief Outro sequence substate update.
 *         This makes the menu and shop icon go out of frame.
 */
static void shop_outro(void)
{
    // Shift the shop panel
    main_bg_se_move_rect_1_tile_vert(POP_MENU_ANIM_RECT, SCREEN_DOWN);

    main_bg_se_copy_rect_1_tile_vert(TOP_LEFT_PANEL_ANIM_RECT, SCREEN_UP);

    if (s_timer == 1)
    {
        tte_erase_rect_wrapper(SHOP_PRICES_TEXT_RECT); // Erase the shop prices text

        ListItr itr = list_itr_create(&s_shop_items_list);
        SpriteObject* shop_item;
        while ((shop_item = list_itr_next(&itr)))
        {
            if (shop_item != NULL)
            {
                shop_item->ty = int2fx(160);
            }
        }

        if (s_shop_voucher != NULL)
            s_shop_voucher->ty = int2fx(160);

        reset_top_left_panel_bottom_row();
    }
    else if (s_timer == 2)
    {
        // TODO: make heads or tails of what's going on here and replace
        // magic numbers.
        int y = 5;
        memset16(&se_mat[MAIN_BG_SBB][y - 1][0], 0x0001, 1);
        memset16(&se_mat[MAIN_BG_SBB][y - 1][1], 0x0002, 7);
        memset16(&se_mat[MAIN_BG_SBB][y - 1][8], SE_HFLIP | 0x0001, 1);
    }

    if (s_timer >= MENU_POP_OUT_ANIM_FRAMES)
    {
        game_change_state(GAME_STATE_BLIND_SELECT);
    }
}

/**
 * @brief Cycling shop lights animation substate update.
 */
static inline void shop_lights_anim_frame(void)
{
    // Shift palette around the border of the shop icon
    COLOR shifted_palette[4];
    shifted_palette[0] = pal_bg_mem[SHOP_LIGHTS_2_PAL_IDX];
    shifted_palette[1] = pal_bg_mem[SHOP_LIGHTS_3_PAL_IDX];
    shifted_palette[2] = pal_bg_mem[SHOP_LIGHTS_4_PAL_IDX];
    shifted_palette[3] = pal_bg_mem[SHOP_LIGHTS_1_PAL_IDX];

    // Circularly shift the palette
    int last = shifted_palette[3];

    for (int i = 3; i > 0; --i)
    {
        shifted_palette[i] = shifted_palette[i - 1];
    }

    shifted_palette[0] = last;

    // Copy the shifted palette to the next 4 slots
    pal_bg_mem[SHOP_LIGHTS_2_PAL_IDX] = shifted_palette[0];
    pal_bg_mem[SHOP_LIGHTS_3_PAL_IDX] = shifted_palette[1];
    pal_bg_mem[SHOP_LIGHTS_4_PAL_IDX] = shifted_palette[2];
    pal_bg_mem[SHOP_LIGHTS_1_PAL_IDX] = shifted_palette[3];
}

void shop_on_update(void)
{
    s_timer++;

    if (s_timer % 20 == 0)
    {
        shop_lights_anim_frame();
    }
}

void shop_on_exit(void)
{
    List* shop_items_list = &s_shop_items_list;
    ListItr itr = list_itr_create(shop_items_list);
    Item* item;

    while ((item = list_itr_next(&itr)))
    {
        item_dispose(&item);
    }

    list_clear(shop_items_list);

    // A Voucher not bought stays offered until the end of the Ante, see voucher_get_shop_offer()
    if (s_shop_voucher != NULL)
        item_dispose(&s_shop_voucher);

    // A Planet used in the Shop leaves its hand's values in the left panel
    hand_clear_level_up_display();

    increment_blind(BLIND_STATE_DEFEATED); // TODO: Move to game_round_end()?

    state_machine_remove(&shop_sm);
}
