from dataclasses import dataclass

@dataclass
class CompileTimeOption:
    key: str
    name: str
    help: str
    define: str
