# sets win32 env for windows on msvc caiwan/IR

function(set_windows_subsystem TARGET)
  if(MSVC)
    target_link_options(${TARGET} INTERFACE "/SUBSYSTEM:WINDOWS")
  endif(MSVC)
endfunction()

function(set_console_subsystem TARGET)
  if(MSVC)
    target_link_options(${TARGET} INTERFACE "/SUBSYSTEM:CONSOLE")
  endif(MSVC)
endfunction()
