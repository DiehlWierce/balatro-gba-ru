# Бюджет памяти

Отчёт снимается после каждой волны: `python3 ru-tools/memory_report.py` (после сборки рома).
Пулы объектов (`include/def_balatro_mempool.h`) показывает `scripts/get_memory_map.sh build/balatro-gba.elf`.

IWRAM — самая тесная память: в ней лежат `.iwram`, `.bss`, `.data` и стек. Новые изменяемые
глобальные данные размером больше нескольких десятков байт кладутся в EWRAM
(`EWRAM_BSS`/`EWRAM_DATA` из tonc или `POOL_ENTRY(..., EWRAM)`), таблицы — `static const` (ROM).

## Волна 0 (база)

| Память | Занято | Всего | % |
|---|---|---|---|
| IWRAM | 27864 | 32768 | 85.0 |
| EWRAM | 3492 | 262144 | 1.3 |
| ROM | 5070380 | 33554432 | 15.1 |

Свободно IWRAM под стек: 4904 байт.
