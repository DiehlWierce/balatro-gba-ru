#include "game/joker_row.h"

#include "blind_effects.h"
#include "consumable.h"
#include "game.h"
#include "game_variables.h"
#include "hand.h"
#include "joker.h"
#include "layout.h"
#include "list.h"
#include "soundbank.h"
#include "sprite.h"
#include "util.h"

/* The owned items row holds the Jokers, then the consumables:
 * index in [0, nb_jokers) is a Joker, index in [nb_jokers, nb_jokers + nb_consumables)
 * is a consumable.
 */

static inline int nb_owned_jokers(void)
{
    return list_get_len(get_jokers_list());
}

Item* owned_items_row_get_item(int idx)
{
    int nb_jokers = nb_owned_jokers();
    if (idx < 0)
        return NULL;
    if (idx < nb_jokers)
        return list_get_at_idx(get_jokers_list(), idx);
    if (idx - nb_jokers < list_get_len(get_consumables_list()))
        return list_get_at_idx(get_consumables_list(), idx - nb_jokers);
    return NULL;
}

int jokers_sel_row_get_size(void)
{
    return nb_owned_jokers() + list_get_len(get_consumables_list());
}

static inline bool is_joker_idx(int idx)
{
    return idx >= 0 && idx < nb_owned_jokers();
}

bool jokers_sel_row_on_selection_changed(
    SelectionGrid* selection_grid,
    int row_idx,
    const Selection* prev_selection,
    const Selection* new_selection
)
{
    List* owned_jokers_list = get_jokers_list();

    // swap Jokers if the A button is held down and both are Jokers on this row
    bool swapping = key_is_down(KEY_CARD_SELECT) && new_selection->y == row_idx &&
                    prev_selection->y == row_idx && is_joker_idx(prev_selection->x) &&
                    is_joker_idx(new_selection->x);

    if (prev_selection->y == row_idx)
    {
        Item* item = owned_items_row_get_item(prev_selection->x);
        // Don't change focus from current Joker if swapping
        if (item != NULL && !swapping)
        {
            sprite_object_erase_text_under((SpriteObject*)item);
            sprite_object_set_focus((SpriteObject*)item, false);
        }
    }

    if (new_selection->y == row_idx)
    {
        Item* item = owned_items_row_get_item(new_selection->x);
        if (item != NULL)
        {
            if (!swapping)
            {
                sprite_object_set_focus((SpriteObject*)item, true);
            }
            // If we land on this row while the A button is being held, we are in swapping mode
            // This means that we need to hide the price, whether we were already
            // on this row or if we come from another
            if (!key_is_down(KEY_CARD_SELECT))
            {
                sprite_object_print_price_under((SpriteObject*)item, item_get_sell_price(item));
            }
        }
    }

    if (swapping)
    {
        list_swap(
            owned_jokers_list,
            (unsigned int)prev_selection->x,
            (unsigned int)new_selection->x
        );
    }

    return true;
}

static inline void sell_joker(int joker_idx)
{
    List* owned_jokers_list = get_jokers_list();

    if (joker_idx < 0 || joker_idx >= list_get_len(owned_jokers_list))
    {
        MGBA_FUNC_ERROR("Invalid joker_idx == %d", joker_idx);
        return;
    }

    Item* joker_object = (Item*)list_get_at_idx(owned_jokers_list, joker_idx);
    blind_effects_on_joker_removed((JokerObject*)joker_object, true);
    item_sell(joker_object);

    remove_owned_joker(joker_idx);
}

/**
 * @brief Planets can be used anytime in the Shop, and only while selecting cards in a round so
 *        the scoring is not changed midway.
 */
static inline bool consumables_can_be_used_now(void)
{
    return game_get_state() != GAME_STATE_ROUND || get_hand_state() == HAND_SELECT;
}

static void consumable_on_key_transit(
    SelectionGrid* selection_grid,
    ConsumableObject* consumable_object
)
{
    // Used when A is released: on press, the release would reach the row below (e.g. select a
    // card in the hand) once the selection moved there
    if (key_released(KEY_CONFIRM) && consumables_can_be_used_now() &&
        consumable_object_can_use(consumable_object))
    {
        // Move the selection away first so the row size is still valid, like when selling
        selection_grid_move_selection_vert(selection_grid, SCREEN_DOWN);
        consumable_object_use(consumable_object);
    }
    else if (key_hit(KEY_SELL))
    {
        selection_grid_move_selection_vert(selection_grid, SCREEN_DOWN);
        consumable_object_sell(consumable_object);
    }
}

void jokers_sel_row_on_key_transit(SelectionGrid* selection_grid, Selection* selection)
{
    if (!is_joker_idx(selection->x))
    {
        ConsumableObject* consumable_object =
            (ConsumableObject*)owned_items_row_get_item(selection->x);
        if (consumable_object != NULL)
            consumable_on_key_transit(selection_grid, consumable_object);
        return;
    }

    JokerObject* joker_object = (JokerObject*)list_get_at_idx(get_jokers_list(), selection->x);
    if (joker_object != NULL)
    {
        if (key_hit(KEY_CARD_SELECT))
        {
            sprite_object_erase_text_under((SpriteObject*)joker_object);
        }
        else if (key_released(KEY_CARD_SELECT))
        {
            sprite_object_print_price_under(
                (SpriteObject*)joker_object,
                joker_object_get_sell_price((Item*)joker_object)
            );
        }
    }

    // Eternal Jokers (Black Stake) can't be sold
    if (key_hit(KEY_SELL) && joker_object != NULL && !joker_can_be_sold(joker_object->joker))
    {
        joker_object_shake(joker_object, SFX_CARD_SELECT);
    }
    else if (key_hit(KEY_SELL))
    {
        int sold_joker_idx = selection->x;

        // Move the selection away from the jokers so it doesn't point to an invalid place
        // Do this before selling the joker so valid row sizes are used
        selection_grid_move_selection_vert(selection_grid, SCREEN_DOWN);

        sell_joker(sold_joker_idx);
    }
}
