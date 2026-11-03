# Balatro GBA — русская версия v0.4.2-ru

Неофициальная русская сборка фанатского порта **Balatro для Game Boy Advance**
([GBALATRO/balatro-gba](https://github.com/GBALATRO/balatro-gba)). База — релиз v0.2.3 и слияние
апстрима до PR #665. Готовый ром — на странице **Releases** этого репозитория.

<p>
<img src="docs/ru/menu.png" width="360"> <img src="docs/ru/deck.png" width="360">
<img src="docs/ru/blind-select.png" width="360"> <img src="docs/ru/game.png" width="360">
<img src="docs/ru/shop.png" width="360"> <img src="docs/ru/shop-pack.png" width="360">
<img src="docs/ru/tarot.png" width="360"> <img src="docs/ru/hand-levels.png" width="360">
<img src="docs/ru/joker-desc.png" width="360"> <img src="docs/ru/options.png" width="360">
<img src="docs/ru/endless.png" width="360"> <img src="docs/ru/game-over.png" width="360">
</p>

## Что переведено

- весь интерфейс: меню, опции, настройка забега, блайнды, раунд, конец раунда, лавка, экраны победы и поражения;
- названия колод, комбинаций, джокеров, боссов, ставок, ваучеров, Таро, спектральных карт, планет и улучшений, описания джокеров;
- заставка с предупреждением; надписи, нарисованные прямо в графике, перерисованы.

Из-за размера тайла 8×8 часть подписей сокращена (МАЛ БЛ., СТРИТ-Ф, РОЯЛ-ФЛ, ФЛ-ХАУС, МАСТ и т. п.).
В графике джокеров остались английские слова на самих картинках. Единые термины — `docs/ru/GLOSSARY.md`.

## Что нового в v0.4.2-ru

Релиз по итогам аудита v0.4.1-ru (`docs/ru/BUGS.md`: 90 находок, исправлены все высокие и средние).
Эталон — Balatro 1.0.x по balatrowiki.org.

**Самое важное**

- **Зависание в середине забега.** В 5-м и поздних раундах громкость звука карт уходила в минус, и звуковой микшер затирал память игры: пропадали кнопки, счёт показывал мусор. Баг был и в v0.4.1-ru. Исправлено, забеги проходят анте 1→8 и бесконечный режим.
- **Обгоревший джокер** снова срабатывает, **Стена и остальные боссы** появляются не раньше своего анте (Бык с 6-го, Змей с 5-го, Растение с 4-го, Глаз и Зуб с 3-го), цель **Иглы** — x1.
- **Колода Анаглиф** (15 колод): после босса выдаёт Двойной тег.
- **Бустеры Джамбо и Мега** и бустер Шутов (15 видов), Негативные расходники в своём слоте, до 12 слотов расходников.
- Расходники в лавке выпадают с весами оригинала; скидки Распродажи и Ликвидации действуют на ваучеры и цену продажи; теги: 18 из 24 (добавлены Uncommon, Rare, Coupon, Orbital, Charm, Meteor, Ethereal, Standard).
- Призрак, Душа, Анх и Невидимый не выходят за массивы при Негативных джокерах; Перкео делает Негативную копию; Аура, Эктоплазма (-1, -2, -3…), Суд и Колесо работают как в оригинале; Таро меняют Каменную карту.
- Кавендиш выпадает только после вымирания Гро Мишеля; Эрозия считает от стартового размера колоды; Подарочная карта поднимает и расходники; Список дел, Рамен, Бейсбольная карточка, Шулер, Безумие, Поднятый кулак поправлены по вики.
- Четыре пальца и Срезка не работают у испорченного джокера; J-Q-K-A одной масти с Четырьмя пальцами — Роял-флеш; Мистер Кость платит за Золотые карты и Синюю печать, но блайнд при спасении не оплачивается.
- Счёт анимируется без переполнения после 8 388 608; Иллюзия: у игральных карт в лавке бывает издание.

Полный список: `docs/ru/BUGS.md`. Новые хост-тесты: `blind_rules` и дополнения к тестам лавки, изданий, спектральных карт, Таро, колод, джокеров.

**Что реализовано в целом:** 15 колод, 8 ставок с наклейками, 28 боссов, 149 джокеров (ID 0–148), издания, улучшения и печати карт, 22 Таро, 18 спектральных карт, планеты и уровни рук, ваучеры, 18 тегов, бустеры всех видов, бесконечный режим до 12-го анте.

**Отличия от оригинала, которые вынуждает железо GBA** (подробно в `docs/ru/STATUS.md`): рубашка Анаглифа взята от синей колоды; Фиолетовая печать рисуется тёмно-синей (в палитре колоды нет фиолетового); множители целые (дробные X1.5 округляются на каждом шаге); не все теги и бустеры в общем ряду лавки; руки не опускаются ниже 1.

## Что нового в v0.4.1-ru

Исправления относительно v0.4.0-ru:

- игра больше не зависает, если слово в тексте шире своей рамки;
- теги за пас не пропадают и не меняют картинку, когда убирают тег из середины;
- Доска и Поднятый кулак не читают уже сыгранные карты, Каменные карты не считаются для Чётного Стивена и Поднятого кулака;
- Обелиск не сбрасывается, если рука делит первое место по частоте с другой;
- Колесо (босс) учитывает «Ой, все шестёрки»; после реролла босса (ваучеры Режиссёрская и Реткон) тот же босс не выпадает снова;
- Вечный Мистер Кости больше не спасает забег бесконечно; Кровавик и Костёр стали необычными;
- Фамильяр, Мрачный и Заклинание не создают Каменные карты;
- шанс Негатива 0,3% не растёт с Заточкой и Блеском;
- скидки в лавке считаются как в оригинале, расходники продаются минимум за $1;
- при долге в конце раунда больше не начисляются лишние деньги; после выхода из победы в меню колода и теги не переносятся в новый забег;
- исправлены выход за границы массива при выборе каре, утечка слота спрайта и мелкие ошибки в графике.

## Что нового в v0.4.0-ru

Относительно v0.3.0-ru:

- **Джокеры.** 149 джокеров (ID 0–148): партии 4–8 добавили 55 новых (ID 94–148), в том числе
  легендарных (Канио, Трибулет, Йорик, Чикот, Перкео), Ретро, Костёр, Невидимого, Матадора и Мима. Подробный список — в `docs/ru/STATUS.md`.
- **Колоды.** 14 колод: к шести прежним добавились Волшебная, Туманность, Призрачная, Заброшенная,
  Клетчатая, Зодиак, Плазменная и Хаотичная.
- **Издания.** Фольга, Голограмма, Полихром и Негатив на джокерах и картах, Колесо фортуны (Таро),
  ваучеры Заточка и Блеск.
- **Спектральные карты.** Спектральные расходники и спектральный бустер, Небесный бустер (планеты),
  Колесо фортуны, ваучеры Шар знамений, Телескоп и Обсерватория.
- **Таро.** Все 22 карты Таро.
- **Ваучеры.** Торговцы и Магнаты Таро и планет, Фокус и Иллюзия и другие.
- **Бесконечный режим.** После победы над боссом 8-го анте можно продолжить забег до анте 12.
  Победа над боссом 12-го анте завершает игру.
- Исправления: размер руки не выходит за массив, Ракета растёт на $2 за босса, испорченный джокер («Скоропорт.») перестаёт
  действовать целиком (Размытый, Ой, все шестёрки, Шоумен, Хаос Клоун), Призрак не обнуляет деньги, если джокер не создан,
  проверки слотов с Негативными джокерами согласованы.

Из ранее добавленного (v0.3.0-ru): 28 боссов, 8 ставок с наклейками, уровни рук и планеты, ваучеры,
улучшения и печати карт, Стандартный бустер и бустер Аркана.

## Чего пока нет

- сохранение забега: в v0.2.3 его отключили из-за зависания при старте (#582);
- бесконечный режим дальше 12-го анте (нужны очень большие числа: мантисса и экспонента);
- коллекция и достижения;
- теги Voucher, Buffoon, Foil, Holographic, Polychrome, Negative (нет спрайтов и второго слота ваучера);
- Мим и Золотые карты в конце раунда работают не во всех случаях, Splash и кап Мима не реализованы (см. `docs/ru/STATUS.md`);
- описания Таро и предметов во время раунда: они показываются только в лавке;
- часть мелких отличий от оригинала описана в разделе «Известные замечания» файла `docs/ru/STATUS.md`.

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
