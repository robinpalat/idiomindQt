# F6-A — UI Dependency Map & Implementation Plan

## UI Dependency Map

### 1. Navigation Strips (4 horizontal bands)

| Strip | Original function | Script | Qt controller/service | Exists? | QML component |
|---|---|---|---|---|---|
| Aprendiendo | notebook_1/2 tab 1 "Learning" | items_list.sh:128-271 | TopicDataRepository.loadIndex | ✅ Partial | LearningView.qml |
| Aprendidos | notebook_1/2 tab 2 "Learnt" | items_list.sh:130-271 | TopicDataRepository (learnt table) | ✅ Partial | LearntView.qml |
| Notas | notebook_1/2/3 tab "Note" | items_list.sh:146-270 | TopicDataRepository.readNote/writeNote | ✅ | NotesView.qml |
| Administrar | notebook_1/2/3 tab "Manage" | items_list.sh:148-326 | TopicRepository, LearningEngine | ✅ Partial | ManageView.qml |

### 2. Contextual Footer

| Tab | Left action | Right action | Original | Qt controller | Exists? |
|---|---|---|---|---|---|
| Aprendiendo | REPRODUCIR | PRACTICAR | play.sh play_list / strt.sh | AudioPlayerService (stub) / PracticeController | Partial |
| Aprendidos | — | — | (no buttons) | — | OK |
| Notas | — | — | (no buttons) | — | OK |
| Administrar | TOPICS | ACCIÓN CONTEXTUAL | chng.sh / mark_as_learned/edit/delete | TopicRepository / LearningEngine | Partial |

### 3. Tasks Panel

| Element | Original | Script | Qt | Exists? | QML |
|---|---|---|---|---|---|
| Tasks list | _get_list T1..T10 | main.sh:102-152, mngr.sh mkmn | SharedRepository, LearningEngine.classifyTopic | ✅ | TasksPanel.qml |

### 4. Topics Modal

| Element | Original | Script | Qt | Exists? | QML |
|---|---|---|---|---|---|
| Topic list | chng.sh topic selector | items_list.sh:341-368 | TopicRepository.listTopics | ✅ | TopicsModal.qml |
| Select topic | sets $DC_s/tpc | chng.sh | (runtime) | ❌ Placeholder | — |
| New topic | new_topic dialog | cnfg.sh / add.sh | TopicRepository.createTopic | ✅ Partial | — |
| Stats | pg_stats.html | ifs/stats.sh | StatsService | ❌ | — |
| Config | cnfg.sh | cnfg.sh | Settings | ✅ Partial | — |

### 5. Play Dialog

| Element | Original | Script | Qt | Exists? | QML |
|---|---|---|---|---|---|
| Play panel | play.sh play_list | play.sh | AudioPlayerService | ❌ Stub | PlayDialog.qml |
| Word/sentence mode | play.sh play_word/play_sentence | play.sh | — | ❌ | — |
| Pause/repeat | pause_rep, loop | play.sh | — | ❌ | — |

### 6. Practice Dialog

| Element | Original | Script | Qt | Exists? | QML |
|---|---|---|---|---|---|
| Mode selector | strt.sh modes 1-5 | practice/strt.sh | PracticeController | ✅ Partial | PracticeDialog.qml |
| Mode 1: Index practice | practice_a | strt.sh | — | ❌ | — |
| Mode 2: Multiple choice | practice_b | strt.sh | — | ❌ | — |
| Mode 3: Flashcards | practice_c | strt.sh | — | ❌ | — |
| Mode 4: Listening | practice_d | strt.sh | — | ❌ | — |
| Mode 5: Audio+replay | practice_e | strt.sh | — | ❌ | — |

### 7. Add Note

| Element | Original | Script | Qt | Exists? | QML |
|---|---|---|---|---|---|
| Add dialog | dlg_form_1/2 | add.sh | AddNoteFlow | ✅ Core | AddDialog.qml |

### 8. Word/Sentence Viewer

| Element | Original | Script | Qt | Exists? | QML |
|---|---|---|---|---|---|
| Viewer | vwr/word_view/sentence_view | vwr.sh | — | ❌ | ViewerDialog.qml |

## Backend Qt Components Status

| Component | Status | Notes |
|---|---|---|
| AppPaths | ✅ Complete | F0 |
| Settings | ✅ Complete | F0 |
| TopicRepository | ✅ Complete | F2 |
| TopicDataRepository | ✅ Complete | F3 |
| ReviewRepository | ✅ Complete | F3 |
| SharedRepository | ✅ Complete | F4 |
| LearningEngine | ✅ Complete | F4 |
| IndexBuilder | ✅ Complete | F4 |
| PracticeLogs | ✅ Complete | F4 |
| PracticeController | ✅ Partial | F4 (no UI) |
| TranslationService | ✅ Complete | F5 |
| TtsPipelineService | ✅ Complete | F5 |
| GrammarColorizer | ✅ Complete | F5 |
| ImageProcessor | ✅ Complete | F5 |
| AddNoteFlow | ✅ Complete | F5 |
| AudioPlayerService | ❌ Not started | Needed for Play |
| StatsService | ❌ Not started | Needed for Stats |

## QML Components Needed for F6-A

| Component | Purpose | Priority |
|---|---|---|
| Main.qml | Root window + navigation | High |
| NavigationStrips.qml | 4 horizontal bands | High |
| ContextualFooter.qml | Footer per strip | High |
| LearningView.qml | Aprendiendo tab content | High |
| LearntView.qml | Aprendidos tab content | High |
| NotesView.qml | Notas tab content | High |
| ManageView.qml | Administrar tab content | High |
| TasksPanel.qml | Expandable tasks overlay | High |
| TopicsModal.qml | Full-screen topic list | High |
| PlayDialog.qml | Playback controls (placeholder) | Medium |
| PracticeDialog.qml | Practice mode selector (placeholder) | Medium |
| ViewerDialog.qml | Word/sentence viewer (placeholder) | Medium |

## F6-A → F6-B Plan

### F6-A (this phase): Pure UI
- QML structure with all components
- Navigation between strips
- Contextual footer per strip
- Tasks panel expand/collapse
- Topics modal open/close
- Play/Practice/Viewer dialogs (placeholder content)
- Transitions and responsive layout
- No backend connections (placeholders for data)

### F6-B (next): Functional integration
- Connect LearningView to TopicDataRepository.loadIndex
- Connect LearntView to learnt table
- Connect NotesView to readNote/writeNote
- Connect ManageView to TopicRepository metadata
- Connect TasksPanel to SharedRepository T1..T10
- Connect TopicsModal to TopicRepository.listTopics
- Connect Play dialog to AudioPlayerService
- Connect Practice dialog to PracticeController
- Connect Add dialog to AddNoteFlow
- Connect Viewer to Item data
