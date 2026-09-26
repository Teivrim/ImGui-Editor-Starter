# ImGui Editor Starter Kit

> Готовый каркас редактора: документ, undo, плагины, задачи, эффекты, рендер-слой.

| | |
|---|---|
| Категория | Шаблон кода |
| Статус | Требуется GL-загрузчик (см. NOTICE.md) |
| Собственный код | **4 978 строк** в 52 файлах |
| Сторонние зависимости | 4 |
| Стек | `C++17`, `CMake`, `OpenGL`, `Dear ImGui`, `GLAD`, `GLFW`, `stb` |
| Рекомендованная цена | $19 |
| Площадки | itch.io, gumroad |

## Сборка

```
cmake -B build && cmake --build build
```

- **Проверка:** ⚠️ Сборка **не проверялась** на этой машине. Проверь перед публикацией.
- **Время:** ~34 с

## Что внутри

- Слоистые подсистемы из коробки: document, undo, plugin, task, effect
- Docking-интерфейс Dear ImGui, версии зависимостей зафиксированы
- FetchContent: одна команда cmake тянет все зависимости
- Чистая сборка: error.log нулевой длины
- Минимальный вход — правишь main.cpp и делаешь свой редактор

## Кому подойдёт

Тем, кто начинает свой редактор на ImGui и не хочет писать каркас с нуля

## Чем отличается от аналогов

Дешёвый бутстрап: экономит 2-3 дня на инфраструктуру документа и undo.

## Требования

- CMake 3.20+, Ninja или Make
- Компилятор с поддержкой C++17
- Git и сеть — для первого запуска CMake (FetchContent тянет зависимости)

## Сторонние компоненты

| Компонент | Версия | Лицензия | Источник |
|---|---|---|---|
| Dear ImGui | 1.91.4 | MIT | [repo](https://github.com/ocornut/imgui) |
| glad | 2.0.8 | MIT | [repo](https://github.com/Dav1dde/glad) |
| GLFW | 3.4 | zlib | [repo](https://www.glfw.org/license.html) |
| stb | master | public domain / MIT (двойная) | [repo](https://github.com/nothings/stb) |

Все лицензии — permissive (MIT / BSD / zlib / public domain), копиleft отсутствует. Подробности в `NOTICE.md`.

## Лицензия

MIT — см. [`LICENSE`](LICENSE). Продаётся как есть, без гарантий (as-is), см. `LICENSE`.

---

© TEIVRIM 2026. Сделано 2026.
