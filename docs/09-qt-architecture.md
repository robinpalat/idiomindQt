# 09 - Arquitectura de la nueva versión — Idiomind Qt 6

> Documento normativo de arquitectura de la reimplementación en **C++20/Qt 6
> (QML)**. Todo comportamiento procede del análisis del original Bash/YAD
> (docs 01-08, 10-13) — **no se añade funcionalidad nueva**.
>
> Legibilidad: cada clase define `Responsibility / Data / Public API /
> Dependencies / Signals / QML exposure`. Las decisiones no deducibles del
> análisis se marcan `DECISIÓN DE DISEÑO`. Se describe navegación, models e
> interfaces QML; **no se diseña la UI pixel a pixel**.

## 0. Derivación y reglas

| Regla | Detalle |
|---|---|
| P1 | El da­ta on-disk es el contrato: `data` (plana ítem), `index` (3 líneas), `stts`, `note`, `translations/*.tra`, `backup/*.bk`, `.idmnd` (JSON 3 líneas). No se cambia nada (docs 03, 05). |
| P2 | El esquema SQLite es el contrato: `tpc`, `reviews`, `config`, `learning`, `learnt`, `words`, `sentences`, `marks`, `Data`, `Pract1..Pract5` (doc 04). Mismo DDL que `mkdb.sh`. |
| P3 | El algoritmo de aprendizaje (doc 06) es lógica **pura** en `core/learning`, validada por tests antes de tocar UI. |
| P4 | Los servicios externos (doc 07) se encapsulan tras interfaces; sin interfaces sin comportamiento. |
| P5 | Los quirk del original se **preservan** (doc 01 § quirks). Se documentan en el propio código/archivo. |
| P6 | Cualquier decisión no deducible → `DECISIÓN DE DISEÑO` explícita. |

## 1. Mapeo Bash → dominio

| Original | Qt |
|---|---|
| `c.conf` vars globales | `AppPaths`, `Settings` (doc 02) |
| `sets.cfg` (`notice[]`, `slangs[]`, `tlangs[]`, `labels_level`, límites frase) | `Settings` (doc 08) |
| ítem plano `trgt{}srce{}...` | `Item` + `FlatItemCodec` (doc 03) |
| ítem JSON `.idmnd` | `Item::toJson/fromJson` (doc 05) |
| info JSON (0..18) | `TopicInfo` (doc 05) |
| `$DC_tlt/note` file | `Note` (doc 03) |
| tabla `reviews` (date1..10) | `Review` (doc 04/06) |
| `mark_as_learned` / `mark_to_learn` / `calculate_review` (mngr.sh) | `LearningEngine` (doc 06) |
| `colorize` (python) genera `index` | `IndexBuilder` (doc 06/03) |
| `check_format_1` + import main.sh | `LegacyImporter` (doc 05) |
| `check_index` (tls.sh) | `TopicRepairService` (doc 04/06) |
| mods/export idmnd | `TopicExporter` (doc 03/05) |
| addons `Resources/scripts/*` | providers `ITranslationProvider`, `ITtsProvider`, `IImageProvider` (doc 07) |
| `sentence_p` + dicts SQLite | `GrammarColorizer` (doc 08) |
| `tts_word/tts_sentence/fetch_audio` | `TtsPipelineService` (doc 07) |
| `img_word`/`set_image` | `ImageDownloader` + `ImageProcessor` (doc 07) |
| `play.sh` + `bucle` | `AudioPlayerService` + `PlaylistEngine` (doc 07) |
| `translate_to` / `transl_batch` | `TranslationService` (doc 07) |
| `stats.sh` → `pg_stats.html` | `StatsService` (doc 10) |
| `itray` python | `TrayController` (doc 11) |
| `clipw` | `ClipWatcherService` (doc 11) |
| `notebook_1..3` / dialogs | vistas QML (doc 11) |

## 2. Capas y dependencias

```
            ┌─────────────────────────────────────────────┐
            │  UI (QML)  ─ Main.qml, NotebookView, ...    │  ← solo presentación
            └──────────────▲──────────────────────────────┘
                           │ QML-facing objects (controllers + models + proxies)
            ┌──────────────┴──────────────────────────────┐
            │  app/controllers  AppController, AddNoteFlow│  orquestación
            │  NavigationController, PracticeController,  │
            │  TrayController                             │
            └──────────────▲──────────────────────────────┘
                           │
            ┌──────────────┴──────────────────────────────┐
            │  services/    interfaces + implementaciones │  efectos externos
            │  (net, audio, ocr, clipboard, updates)      │
            └──────────────▲──────────────────────────────┘
                           │
            ┌──────────────┴──────────────────────────────┐
            │  storage/     repository (SQLite + file)    │  persistencia
            └──────────────▲──────────────────────────────┘
                           │
            ┌──────────────┴──────────────────────────────┐
            │  core/        dominio puro (sin I/O)        │  lógica
            └─────────────────────────────────────────────┘
```

Regla de dependencias: `core` no incluye nada; `storage` incluye `core` +
QtSql; `services` incluye `core` + `storage`; `ui/controllers` incluye
`services`; QML incluye solo controllers+models (nunca core directamente).

## 3. Dominio — `core/`

### 3.1 `Item`
- **Responsibility**: unidad de aprendizaje del topic (palabra u oración). Es un *value object* inmutable con código de ida/vuelta a formato plano y JSON.
- **Data**: `trgt, srce, exmp, defn, note, wrds, grmr, tags, mark, refr, imag, link, cdid, type` + mapa `extraTranslations` (`slch, slde, slen, sles, slfr, slit, slja, slpt, slru, slvi` — ver `.idmnd`, doc 05).
- **Public API**:
  ```cpp
  QString trgt() const; QString srce() const; /* + getters por campo */
  bool marked() const;               // mark == "TRUE"
  ItemType type() const;             // Word | Sentence  (type=="1")
  QString flatLine() const;          // FlatItemCodec::encode
  QJsonObject toJson() const;        // .idmnd style
  static Item fromFlatLine(const QString&);
  static Item fromJson(const QJsonObject&);
  void setNoteText(const QString&);  // edición de nota
  ```
- **Dependencies**: `ItemType` enum (core).
- **Signals**: — (value type).
- **QML exposure**: `Q_GADGET` registrado (`Idiomind.Topic 1.0`), usado como role struct en models; nunca propiedad directa en pantalla salvo vía `ItemsModel`.

### 3.2 `TopicInfo`
- **Responsibility**: metadatos del topic (fila `id` + objeto info del `.idmnd`).
- **Data**: `name, slng, tlng, autr, cntt, ctgy, ilnk, orig, dtec, dteu, dtei, nwrd, nsnt, nimg, naud, nsze, levl, info, stts` (orden = índices 0..18, `tsets[]`).
- **Public API**:
  ```cpp
  QString name() const; /* ... */
  int wordCount() const; int sentenceCount() const; int imageCount() const;
  int audioCount() const; qint64 sizeBytes() const;
  QJsonObject toJson() const;         // .idmnd info line
  static TopicInfo fromJson(const QJsonObject&, bool* ok);
  void recomputeStats(const QList<Item>&);  // nwrd/nsnt/nimg/nsze/naud (doc 03)
  ```
- **Dependencies**: — (comparación con `labels_level` la hace `Settings`).
- **Signals**: — (value type).
- **QML exposure**: `Q_GADGET`; propiedades planas en `TopicListModel` / `tpc_view`.

### 3.3 `Note`
- **Responsibility**: nota de texto libre **a nivel de topic** (fuente: `$DC_tlt/note`; exporta como `info` en `.idmnd`). No confundir con `Item::note` (nota por ítem, doc 03 §Item).
- **Data**: `text`, `modifiedAt`.
- **Public API**: `QString text(); void setText(const QString&); bool isEmpty() const;`
- **Dependencies**: — .
- **Signals**: `noteChanged()`.
- **QML exposure**: propiedad `Note* TopicProxy::note` (línea editable del Info tab).

### 3.4 `Review`
- **Responsibility**: fila `reviews` de un topic (fechas de repaso 1..10) + cómputos derivados de `notice[]`.
- **Data**: `QString date[1..10]` (formato `%m/%d/%Y`), `countDateReviews()`.
- **Public API**:
  ```cpp
  int countFilled() const;              // count_date_reviews
  QString lastDate() const;             // última fecha no vacía
  int daysBetween(const QDate& today) const;   // TM
  int dueInDays() const;                // notice[countFilled]
  double overduePercent() const;        // 100*TM/notice[count]
  void recordReview(const QDate& d);    // escribe la siguiente fecha
  QMap<QString,QString> toRow() const; static Review fromRow(...);
  ```
- **Dependencies**: `Settings::reviewIntervals()` (`notice[]`).
- **Signals**: `reviewRecorded()` (vía TopicProxy).
- **QML exposure**: propiedad `ReviewProxy*` con roles de estado/porcentaje (barra de progreso de intervalo).

### 3.5 `ReviewState` (enum + helpers)
- **Responsibility**: máquina de estados `stts` 0..13 (doc 06, tabla completa).
- **Data**: `enum class State { Paused=0, Learning=1, Mastered=2, WaitingA=3, WaitingB=4, FinalizeA=5, FinalizeB=6, ReadyA=7, ReadyB=8, OverdueA=9, OverdueB=10, Corrupt=13 };`
- **Public API**:
  ```cpp
  static State fromString(const QString&); static QString toString(State);
  bool isFinalizable() const;  // 1,5,6
  bool isReviewingOdd() const; // 3,7,9
  bool isReviewingEven() const;// 4,8,10
  bool isReadyOrOverdue() const; // 7..10
  static State afterMarkAsLearned(State, int count);   // doc 06
  static State afterMarkToLearn(State);                // 5|6
  State transitionForPercent(double p) const;          // doc 06
  ```
- **Dependencies**: —.
- **Signals**: —.
- **QML exposure**: `Q_ENUM` expuesto (statusLabel).

### 3.6 `ReviewCalculator`
- **Responsibility**: lógica **pura** de cálculo de repaso (mngr.sh `calculate_review`, doc 06): lista destino T1..T4/T7/T10 por estado/porcentaje.
- **Data**: `enum class ListKind { T1,T2,T3,T4,T7,T10,None }`.
- **Public API**:
  ```cpp
  static int  daysToReviewAt(int count);             // notice[]
  static int  percentOverdue(int tm, int count);     // 100*TM/due
  static ListKind classify(State st, double p);      // T3/T4/T7/T10
  static State  nextReadyState(State st, double p);  // 7→9 / 8→10 / 3→7 / 4→8 ...
  static bool   isMastered(const Review& r);         // count>=9
  ```
- **Dependencies**: `Settings` (intervals), `ReviewState`.
- **Signals**: —.
- **QML exposure**: — (interna; resultados se ven en `TopicProxy.status`).

### 3.7 `TopicConfig`
- **Responsibility**: fila `config` del topic (doc 04): switches de práctica/reproducción.
- **Data**: `words, sntcs, marks, learn, diffi, rplay, audio, ntosd, loop, rword, acheck, repass` (strings `TRUE/FALSE/`números igual que el original).
- **Public API**: getters/setters booleans; `QString raw(const QString& field)` (para round-trip byte); `int repass() const; bool autoCheck() const;`
- **Dependencies**: —.
- **Signals**: `configChanged(QString field)`.
- **QML exposure**: propiedad del `PreferencesView` per-topic (acualmente config global en `cnfg`; el per-topic vive en Manage tab).

### 3.8 `TopicLists`
- **Responsibility**: contenedor de las listas derivadas del topic (tablas `learning, learnt, words, sentences, marks`, doc 04).
- **Data**: `QVector<Item> learning, learnt, words, sentences, marks`.
- **Public API**: `const QVector<Item>& list(Table) const;` `void rebuildFromItems(const QList<Item>&, bool allLearnt);` (equivale al python `_restore` de check_index, tipo 13/24).
- **Dependencies**: `Item`.
- **Signals**: `listsChanged()`.
- **QML exposure**: fuente de `IndexListModel`/`ItemsModel`.

### 3.9 `Topic`
- **Responsibility**: raíz de agregado del topic en memoria (agrega `TopicInfo + Note + Items + Review + TopicConfig + TopicLists + TranslationSet`). No conoce repositorios ni rutas; el almacenamiento lo aplica `TopicRepository`.
- **Data**: `TopicInfo info_; Note note_; QList<Item> items_; Review review_; TopicConfig config_; TopicLists lists_; ReviewState::State state_; QVector<QString> indexLines_; TranslationSet translations_;`
- **Public API**:
  ```cpp
  QString name() const;
  QList<Item> items() const;
  void setItems(const QList<Item>&);
  bool appendItem(const Item&);         // cap 200 (doc 03)
  bool removeItem(const QString& trgt);
  Item itemByTrgt(const QString&) const;
  ReviewState::State state() const; void setState(ReviewState::State);
  void applyMarkAsLearned();            // delega en LearningEngine (ver 3.11)
  void applyMarkToLearn();
  Review review() const; void setReview(const Review&);
  TopicConfig config() const; void setConfig(const TopicConfig&);
  const QVector<QString>& indexLines() const; void setIndexLines(...);
  ```
- **Dependencies**: `Item, TopicInfo, Note, Review, TopicConfig, TopicLists, TranslationSet, ReviewState`.
- **Signals**: `itemsChanged(), stateChanged(), listsChanged(), indexChanged()`.
- **QML exposure**: no; se expone `TopicProxy` (ver 5) con `@QmlElement` enlazado al topic activo.

### 3.10 `TranslationSet`
- **Responsibility**: traducciones alternativas del topic (carpeta `translations/`): copias `.tra`, `active`, backups `.bk` (doc 03/07).
- **Data**: `QString active; QStringList languages; QMap<QString/*lang*/, QList<Item>> items;`
- **Public API**: `bool isActive(QString lang) const; void setActive(const QString&); bool hasVerified(QString lang) const; bool hasBackup(QString lang) const; QList<Item> itemsOf(QString lang) const;`
- **Dependencies**: `Item`.
- **Signals**: `translationSetChanged()`.
- **QML exposure**: fuente de `TranslationsModel`.

### 3.11 `LearningEngine`
- **Responsibility**: orquesta el ciclo de aprendizaje de un topic (mngr.sh): `mark_as_learned`, `mark_to_learn`, `calculate_review`, actualización de listas y regeneración del índice. **Es la pieza que debe pasar byte-equivalence con Bash.**
- **Data**: referencia a `Topic` en estado de trabajo; resultados intermedios en struct `ReviewPlan { ListKind list; State targetState; double percent; }`.
- **Public API**:
  ```cpp
  void setTopic(Topic*);
  ReviewPlan planForCurrentState() const;              // calculate_review
  void markAsLearned();                                // stts∈{1,5,6} (doc 06)
  void markToLearn();
  void refreshLists();                                 // recompute words/sentences/...
  void setAutoCheck(bool);
  void recordFailure(const QString& trgt, int log);    // log1..log3 (doc 10)
  void clearLogs();
  ```
- **Dependencies**: `Topic`, `ReviewCalculator`, `ReviewState`, `TopicLists`.
- **Signals**: `planChanged(const ReviewPlan&), stateAdvanced(ReviewState::State), logsUpdated()`.
- **QML exposure**: a través de `TopicProxy` (barra de progreso, botones Review/To learn).

### 3.12 `IndexBuilder`
- **Responsibility**: equivalente de `colorize` (python, doc 06/03): genera las 3 líneas del índice desde `learning`, `marks`, `log1..log3`, con spans Pango y flag `TRUE/FALSE`.
- **Data**: entrada `TopicLists*`, `logs`, `bool autoCheck chk`; salida `QVector<QString> lines`.
- **Public API**: `QVector<QString> build(const TopicLists&, const PracticeLogs&, bool chk) const;` con exactos colores (`#AE3259` log3, `#C15F27` log2, `<b><big>` para marks).
- **Dependencies**: `Item, TopicLists`.
- **Signals**: —.
- **QML exposure**: su salida alimenta `IndexListModel`. *(DECISIÓN DE DISEÑO: no parseamos spans en C++; el model expone `display`, `check`, `translation` por separado además del texto span para compatibilidad.)*

### 3.13 `PracticeLogs`
- **Responsibility**: los tres logs de práctica (`$DC_tlt/practice/log1..log3`, doc 10).
- **Data**: `QStringList log1, log2, log3`.
- **Public API**: `bool inLog(int n, const QString& trgt) const; void append(int n, ...); void clearAll();`
- **Dependencies**: —.
- **Signals**: `logsChanged()`.
- **QML exposure**: — (interno del IndexBuilder/color).

### 3.14 `Settings` (config global)
- **Responsibility**: `sets.cfg` + `cnfg` predeterminados: intervalos `notice[]`, mapas `slangs[]/tlangs[]`, `labels_level`, límites de frase (`sentence_lines=2`, `sentence_chars=180`, `sentence_words_level0=12`), parámetros espeak, `useragent`, versión.
- **Data**: structs const + `current` overrides (intrface, synth, tlang, level...).
- **Public API**:
  ```cpp
  QList<int> reviewIntervals() const;             // notice[]
  QString languageNameToCode(const QString& name) const;  // slangs/tlangs
  int sentenceChars() const; int sentenceWordsLevel0() const; int sentenceLines() const;
  QString userAgent() const; QString version() const;
  QList<QString> supportedCategories() const;     // Categories[]
  void loadFromResource(); void save();
  ```
- **Dependencies**: `AppPaths` (for saved config).
- **Signals**: `settingsChanged()`.
- **QML exposure**: propiedad qmlSingleton `SettingsProxy` para páginas de preferencias.

### 3.15 `AppPaths`
- **Responsibility**: reproducción de las variables de `c.conf` como rutas calculadas (doc 02). Compatible *y* multiplataforma.
- **Data**: `dt, ds, dm, dmTl, dmTls, dc, dcA, dcD, cfgDb, sharedDb, tlngDb, topicConfDir(name)→tpcdb`. Con `ReadOnly path` `DS` y `writable` `DM/DC`.
- **Public API**: `QString tmpDir() const; ... QString topicConf(QString topic) const; QString topicData(QString topic) const; QUrl resourceBase() const;`
- **Dependencies**: QtStandardPaths; *no* `KConfig`.
- **Signals**: —.
- **QML exposure**: — (service de plataforma). *(DECISIÓN DE DISEÑO: en sistemas existentes de Bash usamos `$HOME/.idiomind` y `$HOME/.config/idiomind` literalmente para coexistir; en sistemas nuevos usamos `QStandardPaths::ConfigLocation` + `AppDataLocation`; detectamos presencia de legacy al primer arranque.)*

## 4. Persistencia — `storage/`

### 4.1 `TopicRepository`
- **Responsibility**: carga/guardado completo de un `Topic` (db `tpcdb` + archivos `data/index/stts/note`), verificación de invariantes y reparación (equivalente `check_index`, doc 04/06).
- **Data**: rutas resueltas por `AppPaths`; conexiones sqlite (sin WAL por compat, doc 05 riesgos); caché en memoria del topic activo.
- **Public API**:
  ```cpp
  QList<QString> availableTopics() const;            // mkmn listado
  std::unique_ptr<Topic> load(const QString& name, bool repair=true);
  bool save(const Topic&);                          // data+index+stts+note
  bool appendItem(const Topic&, const Item&);
  bool updateItem(const Topic&, const Item&);
  bool removeItem(const Topic&, const QString& trgt);
  bool persistReview(const Topic&, const Review&);
  bool persistConfig(const Topic&, const TopicConfig&);
  bool persistLists(const Topic&);                  // learning/learnt/words/sentences/marks
  int  detectCorruption(const Topic&)const;         // invariantes doc 04
  bool repair(Topic&);                              // rebuild en memoria, luego save
  ```
- **Dependencies**: `Topic, Item, Review, TopicConfig, TopicLists, AppPaths, SqliteHelper`.
- **Signals**: `topicSaved(const QString&), topicCorrupted(const QString&)`.
- **QML exposure**: —.

### 4.2 `SharedRepository`
- **Responsibility**: `shrdb` (`$DM_tls/data/config`): tabla `topics` + listas T1..T4/T7/T10 (doc 04/06).
- **Public API**: `void updateList(ReviewCalculator::ListKind, const QString& topic); QStringList listOf(ReviewCalculator::ListKind) const; void upsertTopic(const QString& name, const QVariantMap& stats);`
- **Dependencies**: `AppPaths`, `SqliteHelper`.
- **Signals**: `listsUpdated()`.
- **QML exposure**: —.

### 4.3 `TranslationMemoryRepository`
- **Responsibility**: `tlngdb` (`Words` + columna `<slng>`), caché de traducciones (doc 04/07 §translate).
- **Public API**: `QString lookup(const QString& word, const QString& language) const; void store(const QString& word, const QString& language, const QString& translation);`
- **Dependencies**: `AppPaths`, `SqliteHelper`.
- **Signals**: —.
- **QML exposure**: —.

### 4.4 `ConfigRepository`
- **Responsibility**: `cfgdb` (`opts/lang/updt/config`, doc 02/04).
- **Public API**: `QVariantMap readOpts() const; void writeOpts(const QVariantMap&); QString readLang(const QString& which) const; QDateTime lastUpdateCheck() const; void setUpdateIgnored(bool); bool updateIgnored() const;`
- **Dependencies**: `AppPaths`, `SqliteHelper`.
- **Signals**: `settingsChanged()`.
- **QML exposure**: vía `SettingsProxy`.

### 4.5 `BackupRepository`
- **Responsibility**: `~/.idiomind/backup/<topic>.bk` (formato flat `----- newest/oldest/end`, límites 200 por sección, escritura si el .bk tiene >2 días, doc 03).
- **Public API**: `bool shouldBackup(const QString& topic) const; void snapshot(const QString& topic, const QStringList& dataLines); QList<QString> restoreSlice(bool newest) const;`
- **Dependencies**: `AppPaths`.
- **Signals**: —.
- **QML exposure**: —.

### 4.6 `AttachmentRepository`
- **Responsibility**: carpeta `files/` del topic + generación `att.html` (mkindex, doc 03/11): audio/video/img/txt/`.url` (Youtube → iframe embed); sanitización de nombre `iconv ascii`.
- **Public API**: `bool addFile(const Topic* , const QString& path); QStringList files() const; bool buildAttachmentHtml(const Topic*) const;`
- **Dependencies**: `Topic, AppPaths`.
- **Signals**: `filesChanged()`.
- **QML exposure**: —.

### 4.7 `TranslationStore`
- **Responsibility**: leer/escribir copias de traducción `.tra`/`.bk`/`active` (doc 03/07).
- **Public API**: `QList<QString> available(const Topic*) const; QString active(const Topic*) const; void setActive(const Topic*, const QString&); QList<Item> backupOf(const Topic*, const QString& lang) const; bool saveVerified(const Topic*, const QString& lang, const QList<Item>&);`
- **Dependencies**: `Topic`, `Item`, `FlatItemCodec`, `AppPaths`.
- **Signals**: —.
- **QML exposure**: fuente de `TranslationsModel` (via controller).

### 4.8 `SqliteHelper`
- **Responsibility**: envoltura mínima de `QSqlDatabase` (open, transaction, query helper, `pragma busy_timeout=2000`, sin WAL — doc 05 riesgos).
- **Public API**: `bool open(QString path); QSqlQuery q(const QString& sql, const QVariantList&); bool inTransaction();`
- **Dependencies**: QtSql.
- **Signals**: —.
- **QML exposure**: —.

### 4.9 `FlatItemCodec`
- **Responsibility**: encode/decode ítem ↔ línea plana `trgt{}srce{}...` (doc 03); escape de comillas/apóstrofes (`''`), parse tolare `type` en índice 13 **y** 24 (doc 03 quirk).
- **Public API**: `QString encode(const Item&); bool decode(const QString&, Item* out) const;`
- **Dependencies**: `Item`.
- **Signals**: —.
- **QML exposure**: —.

## 5. Objetos expuestos a QML

Interfaces públicas de la UI (controllers + models + proxies):

### 5.1 `TopicProxy` (`@QmlElement Idiomind.TopicProxy`)
- **Responsibility**: fachada del topic activo para QML (read/write).
- **Data**: `Topic*` interno, `ReviewPlan`, `Note*`.
- **Public API (Q_INVOKABLE/properties)**: `name, statusLabel, state, percentOverdue, itemsModel, indexModel, translationsModel, note, config, review();` slots `applyReview(); markAsLearned(); markToLearn(); refresh(); openWordViewer(trgt); openSentenceViewer(trgt);`
- **Dependencies**: `LearningEngine`, `TopicRepository`, `IndexBuilder`, `NavigationController`.
- **Signals**: `stateChanged(), planChanged(), noteChanged(), indexRebuilt()`.
- **QML exposure**: contexto de `NotebookView`.

### 5.2 `World`/`AppController` (qmlSingleton `App`)
- **Responsibility**: raíz de comandos globales: abrir topic, selector, importar/exportar, preferencias, stats, updates, tray, salida (equivale a dispatching de launcher y tls.sh).
- **Public API**: `openTopic(name); importFile(url); exportTopic(format); openPreferences(); showStats(); checkUpdates(); restoreBackup(topic); exitApp();`
- **Dependencies**: todos los repos + services.
- **Signals**: `appBusy(bool)`, `notification(QString)`, `logged(QtMsgType, QString)`.
- **QML exposure**: propiedad singleton; botones globales del shell.

### 5.3 `NavigationController`
- **Responsibility**: máquina de navegación (StackView) + historial de diálogos (doc 11).
- **Public API (Q_INVOKABLE)**: `push(Page page, const QVariant& context); pop(); replace(...)`. Enums: `Home, Selector, Notebook, AddNote, WordViewer, SentenceViewer, Preferences, TranslatorBatch, Stats, Attachments, About, Update, ImportPicker`.
- **Dependencies**: —.
- **Signals**: `pageChanged(int page, QVariant context)`.
- **QML exposure**: singleton `Nav`.

### 5.4 `AddNoteFlow` (controller)
- **Responsibility**: pipeline de "agregar nota" (mods/add `process`, doc 07/11): clipboard→OCR→translate→tts→image→build item→append→check_index.
- **Public API**:
  ```cpp
  Q_INVOKABLE void begin(const QString& trgt, const QString& topic);
  Q_INVOKABLE void cancel();
  Q_PROPERTY(double progress READ progress ...);
  Q_PROPERTY(bool running ...);
  signals: step(int), progress(double), finished(Item), error(QString);
  ```
- **Dependencies**: `ITranslationProvider, ITtsProvider, IImageProvider, OcrService, TopicRepository, TranslationMemoryRepository`.
- **Signals**: arriba.
- **QML exposure**: progreso en `AddNoteView`.

### 5.5 `PracticeController`
- **Responsibility**: modos 1..5 (doc 10), listas `Pract1..Pract5`, logs y estadísticas por sesión.
- **Public API**: `beginMode(int); submit(const QVariant& answer); nextItem(); finish();` props `mode, currentItem, total, correct`.
- **Dependencies**: `LearningEngine`, `TopicRepository`, `AudioPlayerService`, `PracticeLogs`.
- **Signals**: `itemChanged(), finished(int correct, int total)`.
- **QML exposure**: pantallas de práctica.

### 5.6 `TrayController`
- **Responsibility**: bandeja del sistema (doc 11): menú Add/Play/Stop/Index/Topics/Quit + tasks; refleja `$DT/playlck`.
- **Public API**: `setPlaying(bool); quit();` (QSystemTrayIcon + menú nativo).
- **Dependencies**: `AppController`, `AudioPlayerService`.
- **Signals**: `actionRequested(QString action)`.
- **QML exposure**: no (widget nativo, no QML).

## 6. Modelos Qt (QAbstractListModel)

### 6.1 `IndexListModel`
- **Responsibility**: el índice editable de práctica. Rows = ítems en `learning` (en orden del `index`). Cada row: `display` (texto+spans), `checkState`, `translation` (srce) (docs 03/06).
- **Roles**: `DisplaySpan, DisplayPlain, Check, Translation, ItemTrgt, Color`.
- **Public API**: `void rebuild(const QVector<QString>& indexLines, const QList<Item>& items); Q_INVOKABLE void toggleCheck(int row);`
- **Dependencies**: `IndexBuilder`, `LearningEngine`.
- **Signals**: `dataChanged` (base).
- **QML exposure**: `ListView` del Index tab; checkbox editable guarda a log1 (auto-check per `acheck`).

### 6.2 `TopicListModel`
- **Responsibility**: lista de topics para selector (`chng`), tray y combos: `name, state, lastDate, count, thumbnail, recent`.
- **Roles**: `Name, State, StateLabel, LastReview, Count, Thumb, Recent`.
- **Public API**: `void reload(const QList<QVariantMap>&);`  *(DECISIÓN DE DISEÑO: reload total por simplicidad; Bash re-escaneaba por mtime en `mkmn`)*.
- **Dependencies**: `SharedRepository`, `AppPaths`.
- **QML exposure**: `HomeView` / selector.

### 6.3 `ItemsModel`
- **Responsibility**: lista plana de ítems del Manage tab (words/sentences/todos), filtrable por `ItemType`.
- **Roles**: `Trgt, Srce, Mark, Type, Example, Note`.
- **Public API**: `void setItems(const QList<Item>&); Q_INVOKABLE void filterByType(int);`
- **Dependencies**: `Item`.
- **QML exposure**: tabla de Manage.

### 6.4 `TranslationsModel`
- **Responsibility**: idiomas disponibles de traducción + estado (active/verified/backup) (doc 07).
- **Roles**: `Language, IsActive, IsVerified, HasBackup`.
- **Public API**: `void reload(const TranslationSet&);`
- **Dependencies**: `TranslationSet`.
- **QML exposure**: combo "Native Language" de `translate_to` + botón de activar/verificar.

### 6.5 `SentencePartsModel`
- **Responsibility**: desglose coloreado de una oración (grammar colors, doc 08): filas `token, part, color`.
- **Roles**: `Token, Part, Color`.
- **Public API**: `void setParts(const QList<GrammarColorizer::Part>&);`
- **Dependencies**: `GrammarColorizer`.
- **QML exposure**: sub-vista del `SentenceViewer`.

### 6.6 `TasksModel`
- **Responsibility**: tareas del directorio `$DT/tasks` (doc 11 tray).
- **Roles**: `Label, Command`.
- **Dependencies**: `AppPaths`.
- **QML exposure**: menú del tray.

## 7. Servicios externos — `services/`

Interfaces (todas en `services/interfaces/`, `namespace idiomind::svc`):

### 7.1 `ITranslationProvider`
- **Responsibility**: traducir texto (palabra/frase) a un idioma; implementación para la paridad con Google Translate y con la memoria `tlngdb`.
- **Data**: config (url, UA).
- **Public API**:
  ```cpp
  virtual TranslationResult translate(const QString& text, const QString& srcLang, const QString& dstLang) = 0;
  virtual BatchResult translateBatch(const QStringList& items, const QString& src, const QString& dst) = 0;
  struct TranslationResult { QString text; bool ok; };
  ```
- **Dependencies**: `QNetworkAccessManager`, `TranslationMemoryRepository`, `AppPaths` (UA).
- **Signals**: `progress(int done,int total)`.
- **QML exposure**: vía `TranslationServiceProxy` (el diálogo `translate_to`/`transl_batch`).
- **Impl**: `GoogleTranslateProvider` — endpoint `translate.googleapis.com/translate_a/single?client=dict-chrome-ex&sl&tl&dt=t&q`, parseo diferenciado CJK vs latino (doc 07); preserva el *fallback de delimitadores* `~~→||→:→_` de transl_batch en `TranslationService`.

### 7.2 `TranslationService`
- **Responsibility**: orquesta `translate_to` y `transl_batch` (doc 07): verifica internet, listo de palabras, 2 pasadas (índice + palabras, sleep 1), copias `.tra/.bk`, `slng_err` y `active`.
- **Public API**: `void runAuto(Topic*, const QString& lang); Q_INVOKABLE void runBatch(Topic*); void restore(topic, lang);`
- **Dependencies**: `ITranslationProvider`, `TranslationStore`, `FlatItemCodec`, `Settings`, `LearningEngine` (para regenerar índice).
- **Signals**: `stepProgress(double), completed(bool), problem(const QString&)`.
- **QML exposure**: diálogo "Native Language Settings" + progress.

### 7.3 `ITtsProvider`
- **Responsibility**: síntesis de voz online/offline por palabra y frase (doc 07).
- **Public API**:
  ```cpp
  virtual bool synthesize(const QString& text, const QString& lang, const QDir& outDir, QString* outFile) = 0; // escribe <word>.mp3
  ```
- **Impls**: `OnlineTtsDownloadProvider` (script-URL + `sox` a mp3), `OnlineTtsConvertProvider`, `OfflineTtsProvider` (espeak params docs 07). *(DECISIÓN DE DISEÑO: se ejecuta cada script de recursos como proceso, igual que Bash, para compat total; en fase 8 se añaden impl nativas.)*
- **Signals**: —.

### 7.4 `TtsPipelineService`
- **Responsibility**: `tts_word`/`tts_sentence`/`fetch_audio`: orden de prioridad de proveedores + caché `$DM_tls/audio/<word>.mp3` (doc 07) + validación de tamaño >120 y MIME (con el quirk del bug preservado en impl, doc 01).
- **Public API**: `bool ensureWord(const QString& w); bool ensureSentence(const QString& s, const QString& topic); void ensureWordsFromList(const QStringList&, const QStringList& /*CJK second arg*/);`
- **Dependencies**: `ITtsProvider*[]`, `AppPaths`, `ImageProcessor` (no), `Settings` (params).
- **Signals**: `progress()`.
- **QML exposure**: barra de progreso del add.

### 7.5 `IImageProvider`
- **Responsibility**: descarga de imagen para una palabra (doc 07 `img_word`).
- **Public API**: `virtual bool download(const QString& word, const QString& alt, const QDir& out) = 0; QUrl imageFor(const QString& word) const;`
- **Impl**: `ScriptImageDownloader` (ejecuta `*.Script.Download image.*` y valida `file image`).
- **Signals**: —.

### 7.6 `ImageProcessor`
- **Responsibility**: resize/crop centrado a `400x270`, thumb `405x275^`, `-quality 90`, fondo blanco cuando `h*100/w>80` (doc 07). Reemplaza `imagemagick` con QImage.
- **Public API**: `bool resizeToCard(const QImage& in, const QString& out); QPixmap thumbnail(const QString& file);`
- **Dependencies**: QtGui.
- **Signals**: —.

### 7.7 `GrammarColorizer`
- **Responsibility**: `sentence_p` (doc 08): split frases → lookup en diccionario SQLite `$DS/default/dicts/<lgt>` (tablas pronouns, nouns_adjetives, nouns_verbs, conjunctions, prepositions, adverbs, adjetives, verbs) → `Part{token, part, color}` + escritura `grmr`.
- **Public API**: `QList<Part> colorize(const QString& sentence, const QString& dictPath) const; struct Part { QString token; QString part; QString color; };` con builder mode (`$2`) incluido.
- **Dependencies**: `SqliteHelper`.
- **Signals**: —.
- **QML exposure**: vía `SentencePartsModel`.

### 7.8 `OcrService`
- **Responsibility**: OCR de clip/pantalla (doc 08/11). *(DECISIÓN DE DISEÑO: el original usaba `gnome-screenshot -a` + herramienta OCR externa sin fijar binario; aquí se abstrae como `IOcrProvider` con un adaptador tesseract por defecto; sin change de pipeline.)*
- **Public API**: `QString recognize(const QImage& area) const;`
- **Dependencies**: `IClipboard`/`ScreenClip` (platform), `IOcrProvider`.
- **Signals**: `recognized(QString)`.

### 7.9 `ClipWatcherService`
- **Responsibility**: `clipw`/`clipw.sh` (doc 11): vigilar portapapeles 5 min, añadir nota a `$DT/tpe` o dialog de add.
- **Public API**: `void start(QString targetTopic); void stop();`
- **Dependencies**: `QClipboard` (platform), `AddNoteFlow`.
- **Signals**: `noteCaptured(QString)`.

### 7.10 `AudioPlayerService`
- **Responsibility**: reproducción por palabra/frase y bucle (doc 07/10): cola de reproducción, `loop`/`pause_rep`/`pause_osd`, control stop por id.
- **Public API**: `bool playWord(const QString& w, const QString& topic); void playList(const QStringList& files, bool loop); void pause(); void stop(bool all); bool playing() const;`
- **Dependencies**: `QMediaPlayer`, `AppPaths`, `Settings`.
- **Signals**: `stateChanged(bool playing), itemChanged(QString), errored(QString)`.
- **QML exposure**: botones play/stop del Index tab y viewers.

### 7.11 `UpdateService`
- **Responsibility**: `check_updates`/`a_check_updates` (doc 07): fetch `checkversion`, `sort -V` compare, throttling `updt`, `ignr`.
- **Public API**: `void check(bool silent); void ignore();`
- **Dependencies**: `QNetworkAccessManager`, `ConfigRepository`, `Settings`.
- **Signals**: `updateAvailable(QString version), upToDate(), checkFailed(QString)`.

### 7.12 `StatsService`
- **Responsibility**: `stats.sh` (doc 10): agrega contadores (por categoría/nivel/stts/intervalo), genera `pg_stats.html`.
- **Public API**: `QVariantMap compute(const Topic&) const; QString renderHtml(const QVariantMap&) const;`
- **Dependencies**: `Topic`, `Review`, `Settings`, `QtGui` (QTextDocument).
- **Signals**: —.
- **QML exposure**: vía página `Stats` (WebEngine o rich text).

### 7.13 `HtmlViewService`
- **Responsibility**: sustituto de `idiomind-htmlview`/`yad --html` (doc 07/11): abre `att.html`, definición, traducción, `pg_stats.html`. *(DECISIÓN DE DISEÑO: QWebEngineView si está disponible; QTextBrowser como fallback — comportamiento de navegación mínimo idéntico.)*
- **Public API**: `void openUrl(const QUrl&); void openFile(const QString&);`
- **Dependencies**: QtWebEngine (opcional) / QTextBrowser.
- **Signals**: —.

### 7.14 `InternetService`
- **Responsibility**: `internet()`/net checks (doc 07) — conectividad no bloqueante y política "best effort" si offline (misma semántica: el add puede fallar y avisar).
- **Public API**: `bool isConnected() const; void checkWith(const std::function<void(bool)>&);`
- **Dependencies**: `QNetworkInformation`.
- **Signals**: `connectivityChanged(bool)`.

## 8. Plataforma — `platform/`

| Clase | Responsibility | API | QML |
|---|---|---|---|
| `PlatformPathsFactory` | resolver `AppPaths` por SO (Linux legacy vs moderno; Windows/mac) | `AppPaths build()` | — |
| `ScreenClipService` | captura de área (reemplaza `import`/`gnome-screenshot`) | `bool capture(QImage* out)` | — |
| `SysNotifications` | `notify-send` equiv. | `void notify(const QString&, int ms)` | check impl de tray |
| `BrowserLauncher` | `xdg-open` equiv. | `bool openUrl(const QUrl&)` | — |
| `Clipboard` | acceso portapapeles multiplataforma | `QString text()` | sí |

*(DECISIÓN DE DISEÑO: estas capas no existían en Bash salvo como comandos externos; se introducen como abstracciones de plataforma sin cambiar comportamiento.)*

## 9. Navegación (QML)

```
Main.qml (Window)
 └─ StackView
     ├─ HomeView            → TopicListModel (selector chng)
     ├─ NotebookView        → IndexListModel + TopicProxy
     │     ├─ InfoTab       (NoteEditor, TopicInfo, stats)
     │     ├─ IndexTab      (ListView + Practice + Review + Listen)
     │     └─ ManageTab     (Attachments, Export, Backup, Stats, Translation)
     ├─ AddNoteView         → AddNoteFlow (progress + fields)
     ├─ WordViewer / SentenceViewer
     ├─ PreferencesView     → SettingsProxy
     ├─ TranslationBatchView → TranslationService
     └─ AttachmentsView      → HtmlViewService (webview)
```
Rutas: `Nav.push(Page.X, context)` — ver 5.3. Transiciones estándar Qt6. La bandeja y `clipw` viven fuera del StackView.

## 10. Estructura de directorios

```
IdiomindQt/
├── CMakeLists.txt
├── cmake/                      # flags, find helpers, toolchain notas
├── src/
│   ├── app/
│   │   ├── main.cpp
│   │   └── Application.*       # composition root: paths, settings, repos, services, tray
│   ├── core/
│   │   ├── config/  AppPaths.*, Settings.*, LanguageCatalog.*
│   │   ├── topic/   Item.*, TopicInfo.*, Topic.*, Note.*, TopicConfig.*,
│   │   │            TopicLists.*, TranslationSet.*, FlatItemCodec.*, IntegratedQuirk.*
│   │   └── learning/ Review.*, ReviewState.*, ReviewCalculator.*,
│   │                  LearningEngine.*, IndexBuilder.*, PracticeLogs.*
│   ├── storage/
│   │   ├── dbs/     SqliteHelper.*, TopicRepository.*, SharedRepository.*,
│   │   │            TranslationMemoryRepository.*, ConfigRepository.*
│   │   ├── files/   FileStore.*, BackupRepository.*, AttachmentRepository.*,
│   │   │            TranslationStore.*
│   │   └── interchange/ LegacyImporter.*, TopicExporter.*
│   ├── services/
│   │   ├── interface/ ITranslationProvider.*, ITtsProvider.*, IImageProvider.*,
│   │   │              OcrProvider.*, UpdateServiceInterface.*
│   │   └── impl/     GoogleTranslateProvider.*, TranslationService.*,
│   │                  TtsPipelineService.*, ImageDownloader.*, ImageProcessor.*,
│   │                  GrammarColorizer.*, OcrService.*, ClipWatcherService.*,
│   │                  AudioPlayerService.*, UpdateService.*, StatsService.*,
│   │                  HtmlViewService.*, InternetService.*
│   ├── ui/
│   │   ├── models/  IndexListModel.*, TopicListModel.*, ItemsModel.*,
│   │   │            TranslationsModel.*, SentencePartsModel.*, TasksModel.*
│   │   ├── qml/     Main.qml, HomeView.qml, NotebookView.qml, AddNoteView.qml,
│   │   │            WordViewer.qml, SentenceViewer.qml, PreferencesView.qml,
│   │   │            TranslationBatchView.qml, StatsView.qml, AttachmentsView.qml,
│   │   │            delegates/*.qml
│   │   └── controllers/ AppController.*, NavigationController.*,
│   │                    TopicProxy.*, AddNoteFlow.*, PracticeController.*,
│   │                    TrayController.*, TranslationServiceProxy.*
│   └── platform/   PlatformPaths.*, ScreenClip.*, SysNotifications.*,
│                   BrowserLauncher.*, Clipboard.*
├── resources/                  # Qt resources
│   ├── default/  (c.conf param set, sets entries, vars templates, gtkrc legacy)
│   ├── images/  logo.png, bar.png, thumb.png, ...
│   ├── dicts/   <lang>.sqlite (grammar dicts de docs 08)
│   ├── ui/       (qml folder es src/ui/qml; resources de imagenes/css)
│   └── locale/   idiomind_*.qm (msgids del Bash, doc 02)
├── tests/
│   ├── unit/        tst_reviewcalculator.cpp, tst_flatcodec.cpp,
│   │                tst_legacyimporter.cpp, tst_indexbuilder.cpp,
│   │                tst_topicrepository.cpp, tst_grammarcolorizer.cpp
│   ├── integration/ fixture_cases/ (dump topics), golden/ (index,data,stts)
│   └── parity/      harness.sh (Bash vs Qt over same HOME fixture)
├── tools/           gen_fixtures.sh, make_golden.sh
└── README.md
```

*(DECISIÓN DE DISEÑO: `resources/default/` conserva las plantillas/c.conf/sets para trazabilidad; el código lee `AppPaths`/`Settings` no los archivos directamente.)*

## 11. CMake

```cmake
cmake_minimum_required(VERSION 3.24)
project(IdiomindQt VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2
             Sql Network Multimedia Concurrent Svg)
find_package(Qt6 COMPONENTS Test)   # solo tests

qt_standard_project_setup()

qt_add_executable(idiomindqt
    src/app/main.cpp
    src/app/Application.cpp
    ... # todos los .cpp de core|storage|services|ui|platform
)

qt_add_qml_module(idiomindqt
    URI Idiomind
    VERSION 1.0
    QML_FILES
      src/ui/qml/Main.qml
      src/ui/qml/HomeView.qml
      ...
    RESOURCES
      resources/default
      resources/images
      resources/dicts
      resources/ui
      resources/locale
)

target_include_directories(idiomindqt PRIVATE src)
target_link_libraries(idiomindqt PRIVATE
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2
    Qt6::Sql Qt6::Network Qt6::Multimedia Qt6::Concurrent Qt6::Svg)

enable_testing()
add_subdirectory(tests)

# instalación
install(TARGETS idiomindqt RUNTIME DESTINATION bin)
```

Notas:
- `tests/unit` y `tests/integration` se añaden con `qt_add_executable` + `target_link_libraries(... Qt6::Test)` y se registran con `add_test`.
- `tools/parity.sh` se registra como test CTest con timeout.
- `CMAKE_AUTOMOC` + `@QmlElement` exigen `<QML_ELEMENT>` registrado sobre clases con `Q_DECLARE_METATYPE`; el módulo QML `Idiomind` expone `TopicProxy`, models, `SettingsProxy`, `Nav`, `App`.

## 12. Estrategia de tests

| Nivel | Cobertura | Técnica |
|---|---|---|
| **Unit** | `ReviewCalculator` (tabla intervals/transiciones completa del doc 06), `FlatItemCodec` (round-trip, unicode, apóstrofes, índice 13/24), `LegacyImporter` (matriz de invalidez campo a campo, doc 05), `IndexBuilder` (salida byte-exacta), `TopicRepository` (invariantes doc 04, sqlite en memoria), `GrammarColorizer` (mapa de colores doc 08), `StatsService` | QtTest `QVERIFY/QCOMPARE`, input en `src` de fixtures |
| **Golden** | Un topic `.conf` real (con `data`, `index`, `stts`, `reviews`, `translations`) → ejecutar operaciones equivalentes (append, mark, colorize) y comparar bytes contra salida *pre-generada por el Bash original* | `tests/integration/golden/`; regenerables con `tools/make_golden.sh` |
| **Integración** | Importar todos los `.idmnd` reales de `Web/…/public/english/**` y verificar: 3 líneas validación OK, ítems ≤200, `check_index` invariantes, exportación idéntica | fixtures + assertions |
| **Paridad** | Mismo fixture `HOME` para Bash original (`Dev/`) e `idiomindqt`; después de secuencias deterministas (add 10 ítems, mark_as_learned, translate mismo idioma, colorize) diff de `data/index/stts/reviews/translations` | `tools/parity.sh` vía CTest |
| **Reversas** | `TopicRepository` round-trip in/out de topic tras `save/load`; corrupt → `repair` | QTemporaryDir |
| **Propiedades UI** | models (setItemType, toggleCheck, filtrado) | QtTest on models |

Criterio de aceptación Fase incremental: **cero diffs** en `data`/`index`/`stts`/`reviews` para las secuencias cubiertas (excepto fechas/locale; ver riesgos).

## 13. Compatibilidad con Idiomind Bash

1. **On-disk contracto intacto**: rutas `$DM/$DC/$DT`, ficheros y DDL sqlite idénticos; coexiste instalación Bash y Qt sobre los mismos datos (sin WAL, sin migración).
2. **`.idmnd`** como único formato de intercambio; `LegacyImporter`/`TopicExporter` byte-equivalentes (doc 05).
3. **Quirks preservados** deliberadamente (doc 01): línea de 3 líneas del índice, `grep 'audio|mpeg|mp3|'` size-only, splitter 140 bytes, índice 13/24, `nsze` con coma.
4. **Recursos**: primero `$DS=/usr/share/idiomind` si existe (mismos dicts/css), si no bundle Qt (`QFile::exists` ⇄ `qrc:/`). `DECISIÓN DE DISEÑO`.
5. **idioma**: msgids del `locale/` original → `QTranslator`; sin cambio de contexto de traducción.
6. **i18n/format**: se marca `localeAware` para `nsze`/fechas; `QDate` usa `%m/%d/%Y` en DB, `%Y-%m-%d` en `.idmnd` (doc 05).
7. **Paridad**: harness (12) bloquea regresiones.

## 14. Flujos de datos

```
A) ADD NOTE (AddNoteFlow)
   clipboard/OCR/text → parse (clean_7..9) → trgt
   → translate(trgt) via GoogleTranslateProvider (+tlngdb cache)
   → tts_word/fetch_audio → $DM_tls/audio/<w>.mp3
   → img_word → $DM_tls/images/<w>-1.jpg | $DM_tlt/images/<w>.jpg
   → Item{trgt,srce,...,type} → FlatItemCodec → TopicRepository.append
   → TopicRepository.save(data) → check_index invariantes → IndexBuilder → index

B) REVIEW (LearningEngine)
   today+TM → ReviewCalculator.classify(stts, percent)
   → stts transition → TopicRepository.persistReview/persistStts
   → SharedRepository.updateList(T1..T10) → IndexBuilder → colorize

C) IMPORT .idmnd
   file → LegacyImporter (3-line+json+check_format_1)
   → ítems ≤200 → Topic.create → TopicRepository.save
   → check_index repair → colorize → topic list update

D) TRANSLATE TOPIC (TranslationService)
   words.trad_tmp/index.trad_tmp → provider (2 pasadas + sleep 1, fallback delimiters)
   → paste → FlatItemCodec → TranslationStore (.tra/active/.bk) → TopicRepository.save
```

## 15. Riesgos técnicos

| Riesgo | Mitigación |
|---|---|
| **Byte-equivalence** del índice (pango spans, TRUE/FALSE, orden por mtime) | `IndexBuilder` consalida determinista; golden fixtures; harness parity |
| CJK paths (`ja|zh-cn|ru`) cambian tokenización y audio (2º argumento) | lógica replicada literalmente; tests dedicados por idioma |
| Esquema sqlite compartido con Bash sin WAL → bloqueos/`busy` | `pragma busy_timeout=2000` igual que original; sin transacciones largas; tests de concurrencia |
| Google Translate endpoint / TTS inestable | parser igual al de Bash; desserialización tolerante; timeout/reintentos como Bash (`-T 51`) |
| QML RichText vs GTK Pango (colores/spans) | el model expone span-plain + color separado; el visual QML usa colores del span, no re-parsing |
| YAD vs QML: `|`-separated forms, `TRUE/FALSE` checklists, `--always-print-result` | contractos de interacción documentados en doc 11; `TopicProxy` devuelve mismos estados |
| Wayland tray / appindicator | `QSystemTrayIcon` + fallback; detección de `playlck` vía `QFileSystemWatcher` |
| HTML attachments (vídeo/iframe/`.url`) | `HtmlViewService` (WebEngine ≥6.5) con fallback QTextBrowser; misma sanificación de nombre |
| `nsze` "8,6M" coma decimal y fechas | campos string como en el original; no se parsea a float excepto para display |
| i18n: msgids duplicados del gettext | extracción a `.ts`/`.qm` automática; lista de msgids invariante |
| Rendimiento al abrir topic grande (200 ítems, imágenes) | carga perezosa de `imag`/audio; modelos no-cargan binarios |
| Los archivos `.tra` grandes (200 líneas) al traducir topic | pipeline por lotes con progress, igual que Bash (2 pasadas) |

## 16. Plan de implementación (resumen — detalle en doc 12)

1. **F0** scaffolding CMake + `AppPaths`/`Settings` + resources.
2. **F2** `core` completo + unit tests (ReviewCalculator/Item codec/IndexBuilder).
3. **F3** storage: repos + `LegacyImporter`/`TopicExporter` + golden fixtures.
4. **F4** `LearningEngine` + práctica + `SharedRepository` + stats.
5. **F5** services (translation/tts/images/ocr) + `AddNoteFlow`.
6. **F6** QML: `TopicProxy`, models, vistas navegables (UI mínima, no completa).
7. **F7** paridad con Bash (harness) + import de datos reales.
8. **F8** Windows/macOS (platform layer) + ajustes de dependencias.

Criterio de parada por fase: **tests verdes + cero diffs byte en fixtures cubiertos**.