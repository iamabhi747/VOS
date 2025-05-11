from lark import Transformer, Tree, Token

def parse_operand(operand):
    if operand.data == 'reg':
        return {
            'type': 'reg',
            'value': operand.children[0].data
        }
    elif operand.data == 'name':
        return {
            'type': 'name',
            'value': operand.children[0].value
        }
    elif operand.data == 'dereference':
        if isinstance(operand.children[0], Tree):
            return {
                'type': 'dereference',
                'value': {
                    'type': 'reg',
                    'value': operand.children[0].data
                }
            }
        else:
            return {
                'type': 'dereference',
                'value': {
                    'type': 'name',
                    'value': operand.children[0].value
                }
            }
    elif operand.data == 'address':
        return {
            'type': 'address',
            'value': {
                'type': 'name',
                'value': operand.children[0].value
            }
        }
    elif operand.data == 'const':
        if operand.children[0].type == 'INT':
            return {
                'type': 'int',
                'value': operand.children[0].value
            }
        else:
            return {
                'type': 'string',
                'value': operand.children[0].value
            }
    assert False, 'Invalid Operand'

def gd(operands):
    assert len(operands) == 1, 'Invalid number of operands for gd instruction'
    return {
        "iname": 'gd',
        "operand": parse_operand(operands[0]),
    }

def pd(operands):
    assert len(operands) == 1, 'Invalid number of operands for pd instruction'
    return {
        "iname": 'pd',
        "operand": parse_operand(operands[0]),
    }

def lr(operands):
    assert len(operands) == 1, 'Invalid number of operands for lr instruction'
    return {
        "iname": 'lr',
        "operand": parse_operand(operands[0]),
    }

def sr(operands):
    assert len(operands) == 1, 'Invalid number of operands for sr instruction'
    return {
        "iname": 'sr',
        "operand": parse_operand(operands[0]),
    }

def cr(operands):
    assert len(operands) == 1, 'Invalid number of operands for cr instruction'
    return {
        "iname": 'cr',
        "operand": parse_operand(operands[0]),
    }

def bt(operands):
    assert len(operands) == 1, 'Invalid number of operands for bt instruction'
    return {
        "iname": 'bt',
        "operand": parse_operand(operands[0]),
    }

def h(operands):
    # assert len(operands) == 0, 'Invalid number of operands for h instruction'
    return {
        "iname": 'h',
        "operand": None,
    }

def add(operands):
    if (len(operands) == 2):
        return [
            {
                "iname": 'lr',
                "operand": parse_operand(operands[0]),
            },
            {
                "iname": 'ad',
                "operand": parse_operand(operands[1]),
            },
            {
                "iname": 'sr',
                "operand": parse_operand(operands[0]),
            }
        ]
    elif (len(operands) == 1):
        return {
            "iname": 'ad',
            "operand": parse_operand(operands[0]),
        }
    assert False, 'Invalid number of operands for add instruction'

def sub(operands):
    if (len(operands) == 2):
        return [
            {
                "iname": 'lr',
                "operand": parse_operand(operands[0]),
            },
            {
                "iname": 'sb',
                "operand": parse_operand(operands[1]),
            },
            {
                "iname": 'sr',
                "operand": parse_operand(operands[0]),
            }
        ]
    elif (len(operands) == 1):
        return {
            "iname": 'sb',
            "operand": parse_operand(operands[0]),
        }
    assert False, 'Invalid number of operands for sub instruction'

def mul(operands):
    if (len(operands) == 2):
        return [
            {
                "iname": 'lr',
                "operand": parse_operand(operands[0]),
            },
            {
                "iname": 'ml',
                "operand": parse_operand(operands[1]),
            },
            {
                "iname": 'sr',
                "operand": parse_operand(operands[0]),
            }
        ]
    elif (len(operands) == 1):
        return {
            "iname": 'ml',
            "operand": parse_operand(operands[0]),
        }
    assert False, 'Invalid number of operands for mul instruction'

def div(operands):
    if (len(operands) == 2):
        return [
            {
                "iname": 'lr',
                "operand": parse_operand(operands[0]),
            },
            {
                "iname": 'dv',
                "operand": parse_operand(operands[1]),
            }
        ]
    elif (len(operands) == 1):
        return {
            "iname": 'dv',
            "operand": parse_operand(operands[0]),
        }
    assert False, 'Invalid number of operands for div instruction'

def inc(operands):
    assert len(operands) == 1, 'Invalid number of operands for inc instruction'
    return {
        "iname": 'im',
        "operand": parse_operand(operands[0]),
    }

def dec(operands):
    assert len(operands) == 1, 'Invalid number of operands for dec instruction'
    return {
        "iname": 'dm',
        "operand": parse_operand(operands[0]),
    }

def bc(operands):
    assert len(operands) == 1, 'Invalid number of operands for bc instruction'
    return {
        "iname": 'bc',
        "operand": parse_operand(operands[0]),
    }

def _and(operands):
    if (len(operands) == 2):
        return [
            {
                "iname": 'lr',
                "operand": parse_operand(operands[0]),
            },
            {
                "iname": 'an',
                "operand": parse_operand(operands[1]),
            }
        ]
    elif (len(operands) == 1):
        return {
            "iname": 'an',
            "operand": parse_operand(operands[0]),
        }
    assert False, 'Invalid number of operands for and instruction'

def _or(operands):
    if (len(operands) == 2):
        return [
            {
                "iname": 'lr',
                "operand": parse_operand(operands[0]),
            },
            {
                "iname": 'or',
                "operand": parse_operand(operands[1]),
            }
        ]
    elif (len(operands) == 1):
        return {
            "iname": 'or',
            "operand": parse_operand(operands[0]),
        }
    assert False, 'Invalid number of operands for or instruction'

def _not(operands):
    assert len(operands) == 1, 'Invalid number of operands for not instruction'
    return {
        "iname": 'nt',
        "operand": parse_operand(operands[0]),
    }

def shl(operands):
    assert len(operands) == 1, 'Invalid number of operands for shl instruction'
    return [
        {
            'iname': 'sr',
            'operand': {
                'type': 'name',
                'value': 'TEMP',
            }
        },
        {
            'iname': 'lr',
            'operand': parse_operand(operands[0]),
        },
        {
            'iname': 'sr',
            'operand': {
                'type': 'reg',
                'value': 'r2',
            }
        },
        {
            'iname': 'lr',
            'operand': {
                'type': 'name',
                'value': 'TEMP',
            }
        },
        {
            'iname': 'ls',
            'operand': {
                'type': 'reg',
                'value': 'i2',
            }
        }
    ]

def shr(operands):
    assert len(operands) == 1, 'Invalid number of operands for shr instruction'
    return [
        {
            'iname': 'sr',
            'operand': {
                'type': 'name',
                'value': 'TEMP',
            }
        },
        {
            'iname': 'lr',
            'operand': parse_operand(operands[0]),
        },
        {
            'iname': 'sr',
            'operand': {
                'type': 'reg',
                'value': 'r2',
            }
        },
        {
            'iname': 'lr',
            'operand': {
                'type': 'name',
                'value': 'TEMP',
            }
        },
        {
            'iname': 'rs',
            'operand': {
                'type': 'reg',
                'value': 'i2',
            }
        }
    ]

def mov(operands):
    assert len(operands) == 2, 'Invalid number of operands for mov instruction'
    return [
        {
            "iname": 'lr',
            "operand": parse_operand(operands[1]),
        },
        {
            "iname": 'sr',
            "operand": parse_operand(operands[0]),
        }
    ]

def jmp(operands):
    assert len(operands) == 1, 'Invalid number of operands for jump instruction'
    return [
        {
            "iname": 'cr',
            "operand": parse_operand(Tree('reg', [Tree('r1', [])]))
        },
        {
            "iname": 'bt',
            "operand": parse_operand(operands[0]),
        }
    ]

INSTRCTIONMAP = {
    'lr': lr, 'sr': sr, 'cr': cr, 'bt': bt,
    'gd': gd, 'pd': pd, 'h': h,
    'add': add, 'sub': sub, 'mul': mul, 'div': div,
    'inc': inc, 'dec': dec, 'bc': bc,
    'and': _and, 'or': _or, 'not': _not,
    'shl': shl, 'shr': shr,
    'mov': mov, 'jmp': jmp,
}

class ASTBuilder(Transformer):
    def instruction(self, items):
        iname = items[0].data
        operands = items[1:]
        return INSTRCTIONMAP[iname](operands)
    
    def text_block(self, items):
        label = items[0].value
        instructions = []
        for itm in items[2:]:
            if isinstance(itm, list):
                instructions.extend(itm)
            else:
                instructions.append(itm)
        return {
            'label': label,
            'instructions': instructions
        }
    
    def text_section(self, items):
        return {
            'section': 'text',
            'blocks': items
        }
    
    def data_statement(self, items):
        return {
            'name': items[0].value,
            'data': {
                'type': 'int' if items[1].type == 'INT' else 'string',
                'value': items[1].value
            }
        }
    
    def data_section(self, items):
        return {
            'section': 'data',
            'statements': items 
        }
    
    def resw(self, items):
        return {
            'type': 'resw',
            'name': items[0].value,
            'size': items[1].value
        }
    
    def resb(self, items):
        return {
            'type': 'resb',
            'name': items[0].value,
        }
    
    def bss_section(self, items):
        return {
            'section': 'bss',
            'statements': items
        }
    
    def section(self, items):
        return items[-1]
    
    def program(self, items):
        return {
            'sections': items
        }