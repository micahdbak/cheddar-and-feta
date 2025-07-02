New-Item -ItemType Directory -Force -Path ".\build\Debug\"
Copy-Item -Path ".\src\assets\*" -Destination ".\build\Debug\" -Recurse -Force

New-Item -ItemType Directory -Force -Path ".\build\Release\"
Copy-Item -Path ".\src\assets\*" -Destination ".\build\Release\" -Recurse -Force