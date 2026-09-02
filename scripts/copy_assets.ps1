New-Item -ItemType Directory -Force -Path ".\build\bin\Debug\"
Copy-Item -Path ".\assets\*" -Destination ".\build\bin\Debug\" -Recurse -Force

New-Item -ItemType Directory -Force -Path ".\build\bin\Release\"
Copy-Item -Path ".\assets\*" -Destination ".\build\bin\Release\" -Recurse -Force
