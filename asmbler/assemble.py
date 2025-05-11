import math

def resolve_bss_data(bss_section, data_section):
    data_cards = [""]

    data_name_map = {}
    for smt in data_section:
        data = ""
        if smt['data']['type'] == 'int':
            data = f"{int(smt['data']['value']):04d}"
        elif smt['data']['type'] == 'string':
            data = bytes(smt['data']['value'][1:-1], "utf-8").decode("unicode_escape")
            if len(data) % 4 != 0: data += ' ' * (4 - (len(data) % 4))
        offset = (len(data_cards) - 1) * 40 + len(data_cards[-1])
        data_cards[-1] += data
        if len(data_cards[-1]) >= 40:
            data_cards.append(data_cards[-1][40:])
            data_cards[-2] = data_cards[-2][:40]
        data_name_map[smt['name']] = offset

    data_offset = (len(data_cards) - 1) * 40 + len(data_cards[-1])

    bss_name_map = {}
    bss_resb_statements = []
    for smt in bss_section:
        if smt['type'] == 'resw':
            bss_name_map[smt['name']] = data_offset
            data_offset += int(smt['size']) * 4
        elif smt['type'] == 'resb':
            bss_resb_statements.append(smt)
        else:
            assert False, 'Unsupported BSS statement'
    
    bss_name_map['TEMP'] = data_offset
    data_offset += 4

    next_empty_block = math.ceil(data_offset / 40)
    for smt in bss_resb_statements:
        bss_name_map[smt['name']] = next_empty_block * 40
        next_empty_block += 1

    if data_cards[-1] == '': data_cards.pop()

    merged_name_map = {**data_name_map, **bss_name_map}
    for name in merged_name_map:
        merged_name_map[name] = merged_name_map[name] // 4
    merged_name_map['__end__'] = next_empty_block * 40 // 4
    return merged_name_map, data_cards

def resolve_text_block(block):
    new_block = []
    for instruction in block:
        if instruction['operand'] and instruction['operand']['type'] == 'dereference':
            new_block.append({
                'iname': 'lr',
                'operand': instruction['operand']['value'],
            })
            new_block.append({
                'iname': instruction['iname'],
                'operand': {
                    'type': 'reg',
                    'value': 'i1'
                },
            })
        else:
            new_block.append(instruction)
    return new_block

def resolve_const(block):
    new_block = []
    new_data = []
    for instruction in block:
        if instruction['operand'] and (instruction['operand']['type'] == 'int' or instruction['operand']['type'] == 'string'):
            if instruction['operand']['type'] == 'int':
                new_block.append({
                    'iname': 'lr',
                    'operand': {
                        'type': 'name',
                        'value': f'TEMP{len(new_data)}'
                    },
                })
                new_block.append({
                    'iname': instruction['iname'],
                    'operand': {
                        'type': 'reg',
                        'value': 'r1'
                    },
                })
                new_data.append({
                    "name": f"TEMP{len(new_data)}",
                    "data": {
                        'type': 'int',
                        'value': instruction['operand']['value']
                    }
                })
            elif instruction['operand']['type'] == 'string':
                assert len(instruction['operand']['value']) == 4, 'String length must be 4'
                new_block.append({
                    'iname': 'lr',
                    'operand': {
                        'type': 'name',
                        'value': f'TEMP{len(new_data)}'
                    },
                })
                new_block.append({
                    'iname': instruction['iname'],
                    'operand': {
                        'type': 'reg',
                        'value': 'r1'
                    },
                })
                new_data.append({
                    "name": f"TEMP{len(new_data)}",
                    "data": {
                        'type': 'string',
                        'value': instruction['operand']['value']
                    }
                })
        else:
            new_block.append(instruction)
    return new_block, new_data

def reserve_bss_for_address(block):
    new_data = []
    for instruction in block:
        if instruction['operand'] and instruction['operand']['type'] == 'address':
            suboperand = instruction['operand']['value']
            assert suboperand['type'] == 'name', f'Unsupported sub-operand type "{suboperand["type"]}" on address'
            new_data.append({
                "name": f"ADDRESS_{suboperand['value']}",
                "data": {
                    'type': 'int',
                    'value': 0
                }
            })
    return new_data

def assemble(ast):
    assert len(ast['sections']) <= 3, 'Unsupported sections'

    bss_section = None
    data_section = None
    text_section = None

    for section in ast['sections']:
        if section['section'] == 'bss':
            if bss_section != None: assert False, 'Unsupported sections'
            bss_section = section['statements']
        elif section['section'] == 'data':
            if data_section != None: assert False, 'Unsupported sections'
            data_section = section['statements']
        elif section['section'] == 'text':
            if text_section != None: assert False, 'Unsupported sections'
            text_section = section['blocks']
        else:
            assert False, 'Unsupported sections'

    for block in text_section:
        block['instructions'], new_data = resolve_const(block['instructions'])
        for data in new_data:
            data_section.append(data)
        new_data = reserve_bss_for_address(block['instructions'])
        for data in new_data:
            data_section.append(data)

    names, data_cards = resolve_bss_data(bss_section, data_section)
    for name in names:
        print(f"{name}: {names[name]}")
    print("="*20)
    for i in range(len(data_cards)):
        print(f"DC[{i}]: |{data_cards[i]}|")

    labels = {}
    for i, block in enumerate(text_section):
        labels[block['label']] = i

    for block in text_section:
        block['instructions'] = resolve_text_block(block['instructions'])

    instructions = []
    if labels.get('start') == None:
        assert False, 'No start label found'
    
    label_offset_map = {'start': 0}
    instructions.extend(text_section[labels['start']]['instructions'])

    for block in text_section:
        if block['label'] == 'start':
            continue
        label_offset_map[block['label']] = len(instructions)
        instructions.extend(block['instructions'])

    return {
        'names': names,
        'data_cards': data_cards,
        'instructions': instructions,
        'label_offset_map': label_offset_map,
    }