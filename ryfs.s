db "ROOT"

dd 0

times 512 - ($ - $$) db 0

db "FILE"


db "examplefilename.txt"
times 64 - 19 db 0

dd 0

db "Hi!"
db 0

times 1024 - ($ - $$) db 0