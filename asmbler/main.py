import json
from lark import Lark
from astbuilder import ASTBuilder
from assemble import assemble
from linker import link

grammar = ""
with open("grammar.lark", 'r') as file:
    grammar = file.read()

def parse_file(filename):
    with open(filename, 'r') as file:
        code = file.read()
    parser = Lark(grammar, start='program', parser='lalr')
    
    # Parse the code to get a parse tree
    tree = parser.parse(code)
    with open("sample_asm.txt", 'w') as file:
        file.write(str(tree.pretty()))

    ast = ASTBuilder().transform(tree)
    with open('ast.txt', 'w') as f:
        json.dump(ast, f, indent=2)

    meta = assemble(ast)
    with open('meta.txt', 'w') as f:
        json.dump(meta, f, indent=2)

    job = link(meta)
    with open('job.txt', 'w') as f:
        f.write(job)

parse_file("sample.asm")