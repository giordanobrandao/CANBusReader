import sys
import subprocess
import pkg_resources

def install_package(package):
    subprocess.check_call([sys.executable, "-m", "pip", "install", package])

def is_package_installed(package):
    try:
        pkg_resources.get_distribution(package)
        return True
    except pkg_resources.DistributionNotFound:
        return False

def convert_to_cpp(input_file, output_h_file, header_define, file_type):
    try:
        # Verifica se o pacote 'csscompressor' está instalado para minimização de CSS
        if file_type == 'css' and not is_package_installed('csscompressor'):
            print("Instalando o pacote 'csscompressor'...")
            install_package('csscompressor')
            
        if file_type == 'js' and not is_package_installed('jsmin'):
            print("Instalando o pacote 'jsmin'...")
            install_package('jsmin')

        if file_type == 'css':
            import csscompressor
        if file_type == 'js':
            from jsmin import jsmin
            
        with open(input_file, 'r', encoding='utf-8') as file:
            content = file.read()

        # Substituições específicas para HTML
        if file_type == 'html':
            content = content.replace('<link rel="stylesheet" href="main.css">', '<style>\n        %MAIN_CSS%\n    </style>')
            content = content.replace('<script src="main.js"></script>', '<script>\n        %MAIN_JS%\n    </script>')

        # Minimização para CSS
        if file_type == 'css':
            content = csscompressor.compress(content)

        # Minimização para JavaScript
        if file_type == 'js':
            content = jsmin(content)

        cpp_content = f'#ifndef {header_define}\n'
        cpp_content += f'#define {header_define}\n\n'
        cpp_content += f'const char main_{file_type}[] PROGMEM = R"rawliteral(\n'
        cpp_content += content
        cpp_content += '\n)rawliteral";\n\n'
        cpp_content += '#endif\n'

        with open(output_h_file, 'w', encoding='utf-8') as file:
            file.write(cpp_content)

        print(f"Arquivo {output_h_file} gerado com sucesso.")
    except Exception as e:
        print(f"Erro: {e}")

if __name__ == "__main__":
    # Argumentos: caminho do arquivo de entrada, caminho do arquivo de saída, identificador do header, tipo de arquivo (html, css, js)
    if len(sys.argv) == 5:
        convert_to_cpp(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
    else:
        print("Uso: python script.py [input_file] [output_h_file] [header_define] [file_type]")
