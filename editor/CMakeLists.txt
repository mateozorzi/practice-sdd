# Busca archivos .qrc
file(GLOB QRC_FILES "EditorMapas/*.qrc")

# Busca archivos .ui (si los tienes)
file(GLOB UI_FILES "EditorMapas/*.ui")

target_sources(game_editor
    PRIVATE
    # .cpp files
    EditorMapas/main.cpp
    EditorMapas/coordinates.cpp
    EditorMapas/creador.cpp
    EditorMapas/editor.cpp
    EditorMapas/messages.cpp
    EditorMapas/checkpoint.cpp
    
    ${QRC_FILES}
    
    ${UI_FILES}
    
    PUBLIC
    # .h files
    EditorMapas/coordinates.h
    EditorMapas/creador.h
    EditorMapas/editor.h
    EditorMapas/messages.h
    EditorMapas/checkpoint.h
    EditorMapas/Constants.h
)
