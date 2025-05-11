import json

INAME_TO_OPCODE = {
    'gd': 'GD',
    'pd': 'PD',
    'h': 'H ',
    'lr': 'LR',
    'sr': 'SR',
    'cr': 'CR',
    'bt': 'BT',
    'ad': 'AD',
    'sb': 'SB',
    'ml': 'ML',
    'dv': 'DV',
    'im': 'IM',
    'dm': 'DM',
    'bc': 'BC',
    'an': 'AN',
    'or': 'OR',
    'nt': 'NT',
    'rs': 'RS',
    'ls': 'LS',
}

def build_job(id, linked_instructions, data_cards):
    id = f'{id:04d}'
    job = f'$AMJ{id}{200:04d}{200:04d}\n'
    for pc in [linked_instructions[i:i+10] for i in range(0, len(linked_instructions), 10)]:
        job += ''.join(pc) + '\n'
    job += '$DTA\n'
    for dc in data_cards:
        job += dc + '\n'
    job += f'$END{id}'
    return job

def link(assembled):
    names = assembled['names']
    data_cards = assembled['data_cards']
    instructions = assembled['instructions']
    label_offset_map = assembled['label_offset_map']

    # print(json.dumps(instructions, indent=2))
    print('=' * 20)
    print(names)
    print('=' * 20)
    print(label_offset_map)

    linked_instructions = []
    initial_offset = 0

    for i, _ in enumerate(data_cards):
        linked_instructions.append(f'GD{i:02d}')
        initial_offset += 1

    for i, instruction in enumerate(instructions):
        li = INAME_TO_OPCODE[instruction['iname']]
        
        operand = instruction['operand']
        if operand == None:
            li += '  '
        elif operand['type'] == 'reg':
            li += operand['value'].upper()
        elif operand['type'] == 'name':
            if li == 'BT' or li == 'BC':
                label_offset = label_offset_map.get(operand['value'])
                assert label_offset != None, f'Label {operand["value"]} not found'
                label_offset += initial_offset
                assert label_offset <= 99, f'Label {operand["value"]} out of range'
                li += f'{label_offset:02d}'

            else:
                name_offset = names.get(operand['value'])
                assert name_offset != None, f'Name {operand["value"]} not found'
                li += f'{name_offset:02d}'
        elif operand['type'] == 'address':
            suboperand = operand['value']
            assert suboperand['type'] == 'name', f'Unsupported sub-operand type "{suboperand["type"]}" on address'

            name_offset = names.get(suboperand['value'])
            assert name_offset != None, f'Name {suboperand["value"]} not found'

            address_offset = names.get('ADDRESS_' + suboperand['value'])
            assert address_offset != None, f'Address {suboperand["value"]} not found'

            data_card_index = address_offset // 10
            data_card_offset = address_offset % 10
            assert data_card_index < len(data_cards), f'Data card index {data_card_index} out of range'

            data_cards[data_card_index] = data_cards[data_card_index][:data_card_offset*4] + f'{(name_offset * 4):04d}' + data_cards[data_card_index][data_card_offset*4 + 4:]

            li += f'{address_offset:02d}'
        else:
            assert False, f'Unsupported operand type "{operand["type"]}"'

        linked_instructions.append(li)

    
    job = build_job(0, linked_instructions, data_cards)

    return job