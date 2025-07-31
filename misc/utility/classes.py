from dataclasses import dataclass

@dataclass
class CompileTimeOption:
    key: str
    name: str
    default: bool
    help: str
    define: str
