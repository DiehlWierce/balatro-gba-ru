# Balatro GBA — русская версия v0.3.0-ru

Неофициальная русская сборка фанатского порта **Balatro для Game Boy Advance**
([GBALATRO/balatro-gba](https://github.com/GBALATRO/balatro-gba)). База — релиз v0.2.3 и слияние
апстрима до PR #665. Готовый ром — на странице **Releases** этого репозитория.

<p>
<img src="docs/ru/menu.png" width="360"> <img src="docs/ru/deck.png" width="360">
<img src="docs/ru/blind-select.png" width="360"> <img src="docs/ru/game.png" width="360">
<img src="docs/ru/shop.png" width="360"> <img src="docs/ru/shop-pack.png" width="360">
<img src="docs/ru/tarot.png" width="360"> <img src="docs/ru/hand-levels.png" width="360">
<img src="docs/ru/joker-desc.png" width="360"> <img src="docs/ru/options.png" width="360">
<img src="docs/ru/game-over.png" width="360">
</p>

## Что переведено

- весь интерфейс: меню, опции, настройка забега, блайнды, раунд, конец раунда, лавка, экраны победы и поражения;
- названия колод, комбинаций, джокеров, боссов, ставок, ваучеров, Таро, планет и улучшений, описания джокеров;
- заставка с предупреждением; надписи, нарисованные прямо в графике, перерисованы.

Из-за размера тайла 8×8 часть подписей сокращена (МАЛ БЛ., СТРИТ-Ф, РОЯЛ-ФЛ, ФЛ-ХАУС, МАСТ и т. п.).
В графике джокеров остались английские слова на самих картинках.

## Что нового в v0.3.0-ru

- **Колоды.** 6 колод: Красная, Синяя, Жёлтая, Зелёная, Чёрная и Раскрашенная. Выбираются стрелками
  в настройке забега. У каждой свой эффект (например, Зелёная: деньги за неиспользованные руки и сбросы,
  без процентов; Чёрная: 6 слотов джокеров).
- **Боссы.** 28 боссов: 14 с игровой логикой и 14 с эффектами на карты и отрисовку (отключение мастей,
  скрытые карты и т. п.). Описания видны на экране выбора блайнда.
- **Ставки.** 8 ставок, от Белой до Золотой. Ставки меняют цели очков, награды и правила: например, −1 сброс
  (Синяя), аренда и вечные или скоропортящиеся джокеры (Чёрная, Оранжевая, Золотая). Наклейки показаны на джокерах.
- **Уровни рук и планеты.** 12 планет, экран «Уровни рук» (SELECT) с очками и множителями каждой комбинации.
- **Ваучеры.** Один ваучер в лавке на анте, в том числе улучшенные версии. Ваучеры меняют лимиты,
  скидки, сбросы и руки.
- **Улучшения и печати.** 8 улучшений карт (Бонус, Множитель, Дикая, Стеклянная, Стальная, Каменная,
  Золотая, Счастливая) и 4 печати (Золотая, Красная, Синяя, Фиолетовая). Фиолетовая печать создаёт Таро.
- **Таро.** 21 из 22 карт Таро как расходники, режим выбора целей (выбрать карты в руке и использовать).
- **Бустеры.** Стандартный бустер (карты добавляются в колоду) и бустер Аркана (Таро на выбор).
- **Бесконечный режим.** После победы над боссом 8-го анте можно продолжить забег до анте 12.
  Победа над боссом 12-го анте завершает игру. Требования анте 9–12 взяты из оригинала.
- **Джокеры 53–93.** 41 новый джокер (партии 1–3): например, Барон, Сверхновая, Вампир, Маска Мидаса,
  Каменный и Стальной джокеры, Гадалка, Взломщик.

## Чего пока нет

- спектральные карты;
- издания джокеров (Foil, Holo, Polychrome, Negative) и Колесо фортуны (Таро, которому нужны издания);
- небесные бустеры (заглушка, в лавке не работают);
- сохранение забега: в v0.2.3 его отключили из-за зависания при старте (#582);
- коллекция и достижения;
- описания Таро и предметов во время раунда: они показываются только в лавке.

## Как собрать

Нужны Docker и Docker Compose. Из корня репозитория:

```bash
docker compose run --rm gbalatro
```

Первый запуск собирает образ из `Dockerfile.gbalatro` (на базе `devkitpro/devkitarm`) и сам ром.
Результат — `build/balatro-gba.gba`. Его можно открыть в mGBA или записать на картридж.
Сборка без Docker описана в `CONTRIBUTING.md` (нужен devkitARM).

## Как играть

| Кнопка | Действие |
|---|---|
| Стрелки | перемещение курсора, выбор ставки и колоды |
| A | выбрать карту, джокера или кнопку |
| B | назад; удерживать, чтобы увидеть описание |
| L | сыграть выбранные карты |
| R | сбросить выбранные карты (на экране блайнда — перебросить босса, если есть ваучер) |
| SELECT | экран «Уровни рук» |
| START | пропустить заставку, начать игру |

Цель каждого блайнда — набрать нужное число очков за ограниченное число рук. В конце раунда
получаете деньги, в лавке покупаете джокеров, расходники, ваучеры и бустеры. Ставка меняет условия
забега, поэтому начинайте с Белой.

## Прогон в эмуляторе

Скриптовые сценарии для mGBA со скриншотами — в `ru-tools/emu/` (см. `ru-tools/emu/README.md`).
Скриншоты в `docs/ru/` сняты так же: сборка через Docker, затем `ru-tools/emu/run.py` с готовыми сценариями.

## Кредиты

- **Balatro** создан LocalThunk и Playstack. Проект неофициальный, не связан с ними. Купите оригинальную игру.
- **Порт на GBA** — апстрим [GBALATRO/balatro-gba](https://github.com/GBALATRO/balatro-gba), на нём основана эта сборка.
- **Русская локализация и доработки** — автор этого репозитория.
- План работ и состояние — `docs/ru/ROADMAP.md` и `docs/ru/STATUS.md`. Термины — `docs/ru/GLOSSARY.md`.

---

# English

This branch (`ru-translation`) is a Russian localization of the Balatro GBA fan port (based on release v0.2.3). The original project README follows.

---

# Balatro-GBA

[![Build Status](https://img.shields.io/github/actions/workflow/status/cellos51/balatro-gba/build_ci_workflow.yml?style=flat&logo=github&branch=main&label=Builds&labelColor=gray&color=default&maxAge=7200)](https://github.com/cellos51/balatro-gba/actions)
[![Open Issues](https://img.shields.io/github/issues/cellos51/balatro-gba?style=flat&color=red&label=Issues&logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAxNiAxNiIgd2lkdGg9IjE2IiBoZWlnaHQ9IjE2Ij48cGF0aCBmaWxsPSJ3aGl0ZSIgZD0iTTggOS41YTEuNSAxLjUgMCAxIDAgMC0zIDEuNSAxLjUgMCAwIDAgMCAzWiI+PC9wYXRoPjxwYXRoIGZpbGw9IndoaXRlIiBkPSJNOCAwYTggOCAwIDEgMSAwIDE2QTggOCAwIDAgMSA4IDBaTTEuNSA4YTYuNSA2LjUgMCAxIDAgMTMgMCA2LjUgNi41IDAgMCAwLTEzIDBaIj48L3BhdGg+PC9zdmc+)](https://github.com/cellos51/balatro-gba/issues)
[![Pull Requests](https://img.shields.io/github/issues-pr/cellos51/balatro-gba?style=flat&color=indigo&label=Pull%20Requests&logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAxNiAxNiIgd2lkdGg9IjE2IiBoZWlnaHQ9IjE2Ij48cGF0aCBmaWxsPSJ3aGl0ZSIgZD0iTTEuNSAzLjI1YTIuMjUgMi4yNSAwIDEgMSAzIDIuMTIydjUuMjU2YTIuMjUxIDIuMjUxIDAgMSAxLTEuNSAwVjUuMzcyQTIuMjUgMi4yNSAwIDAgMSAxLjUgMy4yNVptNS42NzctLjE3N0w5LjU3My42NzdBLjI1LjI1IDAgMCAxIDEwIC44NTRWMi41aDFBMi41IDIuNSAwIDAgMSAxMy41IDV2NS42MjhhMi4yNTEgMi4yNTEgMCAxIDEtMS41IDBWNWExIDEgMCAwIDAtMS0xaC0xdjEuNjQ2YS4yNS4yNSAwIDAgMS0uNDI3LjE3N0w3LjE3NyAzLjQyN2EuMjUuMjUgMCAwIDEgMC0uMzU0Wk0zLjc1IDIuNWEuNzUuNzUgMCAxIDAgMCAxLjUuNzUuNzUgMCAwIDAgMC0xLjVabTAgOS41YS43NS43NSAwIDEgMCAwIDEuNS43NS43NSAwIDAgMCAwLTEuNVptOC4yNS43NWEuNzUuNzUgMCAxIDAgMS41IDAgLjc1Ljc1IDAgMCAwLTEuNSAwWiI+PC9wYXRoPjwvc3ZnPg==)](https://github.com/cellos51/balatro-gba/pulls)
[![Discussions](https://img.shields.io/github/discussions/cellos51/balatro-gba?style=flat&color=blue&label=Discussions&logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAxNiAxNiIgd2lkdGg9IjE2IiBoZWlnaHQ9IjE2Ij48cGF0aCBmaWxsPSJ3aGl0ZSIgZD0iTTEuNzUgMWg4LjVjLjk2NiAwIDEuNzUuNzg0IDEuNzUgMS43NXY1LjVBMS43NSAxLjc1IDAgMCAxIDEwLjI1IDEwSDcuMDYxbC0yLjU3NCAyLjU3M0ExLjQ1OCAxLjQ1OCAwIDAgMSAyIDExLjU0M1YxMGgtLjI1QTEuNzUgMS43NSAwIDAgMSAwIDguMjV2LTUuNUMwIDEuNzg0Ljc4NCAxIDEuNzUgMVpNMS41IDIuNzV2NS41YzAgLjEzOC4xMTIuMjUuMjUuMjVoMWEuNzUuNzUgMCAwIDEgLjc1Ljc1djIuMTlsMi43Mi0yLjcyYS43NDkuNzQ5IDAgMCAxIC41My0uMjJoMy41YS4yNS4yNSAwIDAgMCAuMjUtLjI1di01LjVhLjI1LjI1IDAgMCAwLS4yNS0uMjVoLTguNWEuMjUuMjUgMCAwIDAtLjI1LjI1Wm0xMyAyYS4yNS4yNSAwIDAgMC0uMjUtLjI1aC0uNWEuNzUuNzUgMCAwIDEgMC0xLjVoLjVjLjk2NiAwIDEuNzUuNzg0IDEuNzUgMS43NXY1LjVBMS43NSAxLjc1IDAgMCAxIDE0LjI1IDEySDE0djEuNTQzYTEuNDU4IDEuNDU4IDAgMCAxLTIuNDg3IDEuMDNMOS4yMiAxMi4yOGEuNzQ5Ljc0OSAwIDAgMSAuMzI2LTEuMjc1Ljc0OS43NDkgMCAwIDEgLjczNC4yMTVsMi4yMiAyLjIydi0yLjE5YS43NS43NSAwIDAgMSAuNzUtLjc1aDFhLjI1LjI1IDAgMCAwIC4yNS0uMjVaIj48L3BhdGg+PC9zdmc+)](https://github.com/cellos51/balatro-gba/discussions)

### Disclaimer

This project is a non-profit fan demake of Balatro for the Game Boy Advance, meant to recreate it as accurately as possible including all the visuals that make it satisfying to play. It is **not affiliated with or endorsed by Playstack or LocalThunk** and **it is not to be sold**.  This version is a **minimal tech-demo**, intended for those who already own and know the official game. Refer to the Balatro wiki below for descriptions of game mechanics and joker effects.  All rights remain with the original holders. 

<a href="https://balatrowiki.org/">
  <img src="https://custom-icon-badges.demolab.com/badge/Balatro%20Wiki-194c84?logo=bigjoker&logoColor=fff" alt="Balatro Wiki" width="155">
</a>

### Please buy the official full version from these sources below:

[![Balatro on Steam](https://custom-icon-badges.demolab.com/badge/Balatro%20on%20Steam-194c84?logo=steam&logoColor=fff)](https://store.steampowered.com/app/2379780/Balatro/)
[![Balatro on Google Play](https://custom-icon-badges.demolab.com/badge/Balatro%20on%20Google%20Play-414141?logo=Google-play&logoColor=fff)](https://play.google.com/store/apps/details?id=com.playstack.balatro.android)
[![Balatro on Apple App Store](https://custom-icon-badges.demolab.com/badge/Balatro%20on%20Apple%20App%20Store-0D96F6?logo=app-store&logoColor=fff)](https://apps.apple.com/us/app/balatro/id6502453075)
[![Balatro on Nintendo eShop](https://custom-icon-badges.demolab.com/badge/Balatro%20on%20Nintendo%20eShop-e60012?logo=nintendo&logoColor=fff)](https://www.nintendo.com/us/store/products/balatro-switch/)
[![Balatro on PlayStation Store](https://custom-icon-badges.demolab.com/badge/Balatro%20on%20PlayStation%20Store-006FCD?logo=PlayStation&logoColor=fff)](https://store.playstation.com/en-us/concept/10010334)
[![Balatro on Xbox](https://custom-icon-badges.demolab.com/badge/Balatro%20on%20Xbox-107C10.svg?logo=xbox&logoColor=white)](https://www.xbox.com/en-US/games/store/balatro/9PK087LNGJC5)
[![Balatro on Humble Bundle](https://img.shields.io/badge/Balatro%20on%20Humble%20Bundle-%23494F5C.svg?logo=HumbleBundle&logoColor=white)](https://www.humblebundle.com/store/balatro?srsltid=AfmBOoqS2De8T4kizzWxJS1pbvQosJ_bYCl4qvC6LA1YLPAh4sZ8vJqO)

---

<a href="https://github.com/cellos51/balatro-gba/releases/">
  <img src="https://img.shields.io/badge/Download_ROMs_from_the_Releases_tab-8A2BE2?&logo=github" alt="Download ROM" width="500">
</a>

See **_CONTRIBUTING.md_** for build instructions for Docker, Linux, MacOS, and Windows.

---


https://github.com/user-attachments/assets/54a9e2e9-1a02-48d5-bb9d-5ab257a7e03b


### Controls:

(A: Pick Card / Make Selection)

(B: Deselect All Cards)

(L: Sell Joker  / Play Hand)

(R: Discard Hand)

(Hold A + D-Pad: Move Cards and Jokers in Hand)

(Hold B: Display Joker Description **_Shop Only_**)

# Contributing

If you would like to contribute, please read CONTRIBUTING.md.

# **Credits:**
## **Game**
This GBA implementation is based on Balatro which is designed and created by LocalThunk and published by Playstack.
See repository contributors list for code contribution credits to this GBA implementation.
## **Music**
Music arrangement is made by @cellos51 and @MeirGavish based on original Balatro soundtrack by [LouisF](https://louisfmusic.com/) and [transcription by MrCrimit](https://musescore.com/user/8237321/scores/14590660).
## **Imagery**
Sprites and backgrounds are based on original Balatro imagery created by LocalThunk.
See [Joker Art Discussion](https://github.com/cellos51/balatro-gba/discussions/69) for full credits for each joker sprite.
## **Sounds**
For the mult and xmult sound effects: [Toy records#06-E3-02.wav by poissonmort](https://freesound.org/s/253249/) used under License: Attribution 4.0

All other sound effects were created by LocalThunk and are used under Creative Commons - CC0 license. 
