#include <LIEF/LIEF.hpp>
#include <LIEF/ELF.hpp>
#include <LIEF/PE.hpp>

#include <capstone/capstone.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

/*
 * @desc: exe - Executable Information Utility
 *
 * Supported formats:
 *   - ELF
 *   - PE
 *
 * Dependencies:
 *   - LIEF
 *   - Capstone
 */

struct Options {
    bool info = false;
    bool sections = false;
    bool addresses = false;
    bool mapping = false;
    bool strings = false;
    bool disassembly = false;

    bool json = false;
    bool pipe = false;

    std::string file;
    std::string output;
};

struct Record {
    std::string type;
    std::vector<std::string> fields;
};

static void print_help()
{
    std::cout <<
        "Usage: exe [OPTIONS] [FILE]\n"
        "\n"
        "Inspect an ELF or PE executable.\n"
        "\n"
        "Options:\n"
        "  -i       Show basic executable information\n"
        "  -s       Show section names, offsets, sizes and flags\n"
        "  -S       Show virtual addresses and file offsets\n"
        "  -m       Map section file offsets to virtual addresses\n"
        "  -j       Output JSON\n"
        "  -P       Output pipe-delimited records\n"
        "  -a       Show all information, strings and disassembly\n"
        "  -f FILE  Write output to FILE\n"
        "  -h       Display this help\n"
        "\n"
        "Exit codes:\n"
        "  0  Success\n"
        "  1  Invalid executable\n"
        "  2  Invalid path\n"
        "  3  Invalid option\n"
        "  4  Invalid option argument\n";
}

static std::string hex_value(uint64_t value)
{
    std::ostringstream out;
    out << "0x" << std::hex << std::uppercase << value;
    return out.str();
}

static std::string decimal_value(uint64_t value)
{
    return std::to_string(value);
}

static std::string json_escape(const std::string& value)
{
    std::ostringstream out;

    for (unsigned char c : value) {
        switch (c) {
            case '"':  out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b";  break;
            case '\f': out << "\\f";  break;
            case '\n': out << "\\n";  break;
            case '\r': out << "\\r";  break;
            case '\t': out << "\\t";  break;

            default:
                if (c < 0x20) {
                    out << "\\u"
                        << std::hex
                        << std::setw(4)
                        << std::setfill('0')
                        << static_cast<unsigned int>(c);
                } else {
                    out << static_cast<char>(c);
                }
        }
    }

    return out.str();
}

static std::string pipe_escape(const std::string& value)
{
    std::string result;

    for (char c : value) {
        switch (c) {
            case '\\': result += "\\\\"; break;
            case '|':  result += "\\|";  break;
            case '\n': result += "\\n";  break;
            case '\r': result += "\\r";  break;
            case '\t': result += "\\t";  break;
            default:   result += c;      break;
        }
    }

    return result;
}

static bool parse_args(
    int argc,
    char** argv,
    Options& opt,
    int& status)
{
    status = 0;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            print_help();
            return false;
        }

        if (arg == "-i") {
            opt.info = true;
        }
        else if (arg == "-s") {
            opt.sections = true;
        }
        else if (arg == "-S") {
            opt.addresses = true;
        }
        else if (arg == "-m") {
            opt.mapping = true;
        }
        else if (arg == "-j") {
            opt.json = true;
        }
        else if (arg == "-P") {
            opt.pipe = true;
        }
        else if (arg == "-a") {
            opt.info = true;
            opt.sections = true;
            opt.addresses = true;
            opt.mapping = true;
            opt.strings = true;
            opt.disassembly = true;
        }
        else if (arg == "-f") {
            if (i + 1 >= argc) {
                std::cerr << "exe: -f requires an output file\n";
                status = 4;
                return false;
            }

            opt.output = argv[++i];

            if (opt.output.empty()) {
                std::cerr << "exe: invalid output filename\n";
                status = 4;
                return false;
            }
        }
        else if (!arg.empty() && arg[0] == '-') {
            std::cerr << "exe: unknown option: " << arg << '\n';
            status = 3;
            return false;
        }
        else {
            if (!opt.file.empty()) {
                std::cerr << "exe: too many input files\n";
                status = 4;
                return false;
            }

            opt.file = arg;
        }
    }

    if (opt.json && opt.pipe) {
        std::cerr << "exe: -j and -P cannot be used together\n";
        status = 3;
        return false;
    }

    if (opt.file.empty()) {
        std::cerr << "exe: missing input file\n";
        status = 4;
        return false;
    }

    if (!opt.info &&
        !opt.sections &&
        !opt.addresses &&
        !opt.mapping &&
        !opt.strings &&
        !opt.disassembly) {
        opt.info = true;
    }

    return true;
}

/*
 * @desc: Identify the executable format.
 */
static std::string format_name(const LIEF::Binary& binary)
{
    switch (binary.format()) {
        case LIEF::Binary::FORMATS::ELF:
            return "ELF";

        case LIEF::Binary::FORMATS::PE:
            return "PE";

        default:
            return "Unknown";
    }
}

/*
 * @desc: Identify the target architecture.
 */
static std::string architecture_name(
    LIEF::Header::ARCHITECTURES arch)
{
    using Arch = LIEF::Header::ARCHITECTURES;

    switch (arch) {
        case Arch::X86:    return "x86";
        case Arch::X86_64: return "x86_64";
        case Arch::ARM:    return "ARM";
        case Arch::ARM64:  return "ARM64";
        case Arch::MIPS:   return "MIPS";
        case Arch::PPC:    return "PowerPC";
        case Arch::PPC64:  return "PowerPC64";
        case Arch::RISCV:  return "RISC-V";
        case Arch::SYSZ:   return "SystemZ";
        case Arch::SPARC:  return "SPARC";
        default:           return "Unknown";
    }
}

/*
 * @desc: Return section permission flags.
 */
static std::string section_flags(const LIEF::Section& section)
{
    std::string flags;

    if (const auto* elf =
            dynamic_cast<const LIEF::ELF::Section*>(&section)) {

        if (elf->has(LIEF::ELF::Section::FLAGS::ALLOC))
            flags += 'R';

        if (elf->has(LIEF::ELF::Section::FLAGS::WRITE))
            flags += 'W';

        if (elf->has(LIEF::ELF::Section::FLAGS::EXECINSTR))
            flags += 'X';
    }
    else if (const auto* pe =
                 dynamic_cast<const LIEF::PE::Section*>(&section)) {

        using Char = LIEF::PE::Section::CHARACTERISTICS;

        if (pe->has_characteristic(Char::MEM_READ))
            flags += 'R';

        if (pe->has_characteristic(Char::MEM_WRITE))
            flags += 'W';

        if (pe->has_characteristic(Char::MEM_EXECUTE))
            flags += 'X';
    }

    return flags.empty() ? "-" : flags;
}

/*
 * @desc: Determine whether a section contains executable code.
 */
static bool is_executable_section(const LIEF::Section& section)
{
    if (const auto* elf =
            dynamic_cast<const LIEF::ELF::Section*>(&section)) {
        return elf->has(LIEF::ELF::Section::FLAGS::EXECINSTR);
    }

    if (const auto* pe =
            dynamic_cast<const LIEF::PE::Section*>(&section)) {
        return pe->has_characteristic(
            LIEF::PE::Section::CHARACTERISTICS::MEM_EXECUTE
        );
    }

    return false;
}

/*
 * @desc: Select Capstone architecture and mode.
 */
static bool select_capstone(
    const LIEF::Header& header,
    cs_arch& arch,
    cs_mode& mode)
{
    using Arch = LIEF::Header::ARCHITECTURES;
    using Modes = LIEF::Header::MODES;

    switch (header.architecture()) {
        case Arch::X86:
            arch = CS_ARCH_X86;

            if (header.is(Modes::BITS_16))
                mode = CS_MODE_16;
            else
                mode = CS_MODE_32;

            return true;

        case Arch::X86_64:
            arch = CS_ARCH_X86;
            mode = CS_MODE_64;
            return true;

        case Arch::ARM:
            arch = CS_ARCH_ARM;
            mode = header.is(Modes::THUMB)
                ? CS_MODE_THUMB
                : CS_MODE_ARM;

            if (header.endianness() ==
                LIEF::Header::ENDIANNESS::BIG) {
                mode = static_cast<cs_mode>(
                    mode | CS_MODE_BIG_ENDIAN
                );
            }

            return true;

        case Arch::ARM64:
            arch = CS_ARCH_ARM64;
            mode = CS_MODE_ARM;

            if (header.endianness() ==
                LIEF::Header::ENDIANNESS::BIG) {
                mode = static_cast<cs_mode>(
                    mode | CS_MODE_BIG_ENDIAN
                );
            }

            return true;

        default:
            return false;
    }
}

/*
 * @desc: Add basic executable information.
 */
static void collect_info(
    const LIEF::Binary& binary,
    uint64_t file_size,
    std::vector<Record>& records)
{
    const auto header = binary.header();

    records.push_back({
        "info",
        {"Format", format_name(binary)}
    });

    records.push_back({
        "info",
        {"Architecture", architecture_name(header.architecture())}
    });

    records.push_back({
        "info",
        {"Bits", header.is_64() ? "64" :
                 header.is_32() ? "32" : "Unknown"}
    });

    records.push_back({
        "info",
        {"Entry point", hex_value(binary.entrypoint())}
    });

    records.push_back({
        "info",
        {"Image base", hex_value(binary.imagebase())}
    });

    records.push_back({
        "info",
        {"File size", decimal_value(file_size)}
    });

    records.push_back({
        "info",
        {"Position independent", binary.is_pie() ? "Yes" : "No"}
    });

    records.push_back({
        "info",
        {"NX protection", binary.has_nx() ? "Yes" : "No"}
    });
}

/*
 * @desc: Collect section information.
 */
static void collect_sections(
    const LIEF::Binary& binary,
    std::vector<Record>& records)
{
    for (const LIEF::Section& section : binary.sections()) {

        if (section.name().empty())
            continue;

        if (section.size() == 0)
            continue;

        records.push_back({
            "section",
            {
                section.name(),
                hex_value(section.offset()),
                decimal_value(section.size()),
                hex_value(section.virtual_address()),
                section_flags(section)
            }
        });
    }
}

/*
 * @desc: Map the start of each file-backed section to its VA.
 *
 * This is a section mapping report, not a guarantee that every
 * file offset belongs to a loadable memory region.
 */
static void collect_mappings(
    const LIEF::Binary& binary,
    std::vector<Record>& records)
{
    for (const LIEF::Section& section : binary.sections()) {
        if (section.size() == 0 ||
            section.virtual_address() == 0)
            continue;

        records.push_back({
            "mapping",
            {
                section.name(),
                hex_value(section.offset()),
                hex_value(section.virtual_address())
            }
        });
    }
}

/*
 * @desc: Extract printable ASCII and UTF-16 strings.
 */
static void collect_strings(
    const std::vector<uint8_t>& data,
    std::vector<Record>& records)
{
    constexpr size_t MIN_LENGTH = 4;

    auto printable = [](uint32_t c) -> bool {
        return c >= 0x20 && c <= 0x7e;
    };

    // ASCII strings
    {
        std::string current;
        uint64_t start = 0;

        for (size_t i = 0; i < data.size(); ++i) {
            unsigned char c = data[i];

            if (printable(c)) {
                if (current.empty())
                    start = i;

                current += static_cast<char>(c);
            }
            else {
                if (current.size() >= MIN_LENGTH) {
                    records.push_back({
                        "string",
                        {
                            hex_value(start),
                            "ASCII",
                            current
                        }
                    });
                }

                current.clear();
            }
        }

        if (current.size() >= MIN_LENGTH) {
            records.push_back({
                "string",
                {
                    hex_value(start),
                    "ASCII",
                    current
                }
            });
        }
    }

    // UTF-16 strings
    auto extract_utf16 = [&](bool big_endian) {
        std::string current;
        uint64_t start = 0;

        for (size_t i = 0; i + 1 < data.size(); i += 2) {
            uint16_t c;

            if (big_endian) {
                c = (static_cast<uint16_t>(data[i]) << 8) |
                    static_cast<uint16_t>(data[i + 1]);
            }
            else {
                c = static_cast<uint16_t>(data[i]) |
                    (static_cast<uint16_t>(data[i + 1]) << 8);
            }

            if (c == 0) {
                if (current.size() >= MIN_LENGTH) {
                    records.push_back({
                        "string",
                        {
                            hex_value(start),
                            big_endian ? "UTF-16BE" : "UTF-16LE",
                            current
                        }
                    });
                }

                current.clear();
                continue;
            }

            if (printable(c)) {
                if (current.empty())
                    start = i;

                current += static_cast<char>(c);
            }
            else {
                if (current.size() >= MIN_LENGTH) {
                    records.push_back({
                        "string",
                        {
                            hex_value(start),
                            big_endian ? "UTF-16BE" : "UTF-16LE",
                            current
                        }
                    });
                }

                current.clear();
            }
        }

        if (current.size() >= MIN_LENGTH) {
            records.push_back({
                "string",
                {
                    hex_value(start),
                    big_endian ? "UTF-16BE" : "UTF-16LE",
                    current
                }
            });
        }
    };

    extract_utf16(false);
    extract_utf16(true);
}

/*
 * @desc: Convert bytes to a readable hexadecimal string.
 */
static std::string bytes_to_hex(
    const uint8_t* bytes,
    size_t size)
{
    std::ostringstream out;

    for (size_t i = 0; i < size; ++i) {
        if (i != 0)
            out << ' ';

        out << std::hex
            << std::uppercase
            << std::setw(2)
            << std::setfill('0')
            << static_cast<unsigned int>(bytes[i]);
    }

    return out.str();
}

/*
 * @desc: Disassemble executable sections using Capstone.
 */
static bool collect_disassembly(
    const LIEF::Binary& binary,
    std::vector<Record>& records)
{
    cs_arch arch;
    cs_mode mode;

    if (!select_capstone(binary.header(), arch, mode))
        return false;

    csh handle;

    if (cs_open(arch, mode, &handle) != CS_ERR_OK)
        return false;

    cs_option(handle, CS_OPT_DETAIL, CS_OPT_OFF);

    for (const LIEF::Section& section : binary.sections()) {
        if (!is_executable_section(section))
            continue;

        auto content = section.content();

        if (content.size() == 0)
            continue;

        cs_insn* instructions = nullptr;

        size_t count = cs_disasm(
            handle,
            content.data(),
            content.size(),
            section.virtual_address(),
            0,
            &instructions
        );

        for (size_t i = 0; i < count; ++i) {
            const cs_insn& insn = instructions[i];

            records.push_back({
                "instruction",
                {
                    hex_value(insn.address),
                    bytes_to_hex(insn.bytes, insn.size),
                    insn.mnemonic,
                    insn.op_str
                }
            });
        }

        cs_free(instructions, count);
    }

    cs_close(&handle);
    return true;
}

/*
 * @desc: Render human-readable output.
 */
static void print_human(
    std::ostream& out,
    const std::vector<Record>& records)
{
    for (const Record& record : records) {
        if (record.type == "info") {
            out << std::left
                << std::setw(24)
                << (record.fields[0] + ":")
                << record.fields[1]
                << '\n';
        }
        else if (record.type == "section") {
            out << "\nSECTION "
                << record.fields[0]
                << "\n"
                << "  Offset: " << record.fields[1]
                << "\n"
                << "  Size:   " << record.fields[2]
                << "\n"
                << "  VA:     " << record.fields[3]
                << "\n"
                << "  Flags:  " << record.fields[4]
                << '\n';
        }
        else if (record.type == "mapping") {
            out << "MAP "
                << record.fields[0]
                << "  "
                << record.fields[1]
                << " -> "
                << record.fields[2]
                << '\n';
        }
        else if (record.type == "string") {
            out << record.fields[0]
                << "  ["
                << record.fields[1]
                << "] "
                << record.fields[2]
                << '\n';
        }
        else if (record.type == "instruction") {
            out << record.fields[0]
                << "  "
                << std::left
                << std::setw(28)
                << record.fields[1]
                << std::setw(12)
                << record.fields[2]
                << record.fields[3]
                << '\n';
        } else if (record.type == "address") {
            out << std::left
                << std::setw(24) << record.fields[0]
                << std::setw(18) << record.fields[1]
                << record.fields[2]
                << '\n';
        }
    }
}

/*
 * @desc: Render JSON output.
 */
static void print_json(
    std::ostream& out,
    const std::vector<Record>& records)
{
    out << "{\n  \"records\": [\n";

    for (size_t i = 0; i < records.size(); ++i) {
        const Record& record = records[i];

        out << "    {\n"
            << "      \"type\": \""
            << json_escape(record.type)
            << "\",\n"
            << "      \"fields\": [";

        for (size_t j = 0; j < record.fields.size(); ++j) {
            if (j != 0)
                out << ", ";

            out << '"'
                << json_escape(record.fields[j])
                << '"';
        }

        out << "]\n    }";

        if (i + 1 != records.size())
            out << ',';

        out << '\n';
    }

    out << "  ]\n}\n";
}

/*
 * @desc: Render pipe-delimited output.
 */
static void print_pipe(
    std::ostream& out,
    const std::vector<Record>& records)
{
    for (const Record& record : records) {
        out << pipe_escape(record.type);

        for (const std::string& field : record.fields)
            out << '|' << pipe_escape(field);

        out << '\n';
    }
}

/*
 * @desc: Collect section virtual addresses and file offsets.
 */
static void collect_addresses(
    const LIEF::Binary& binary,
    std::vector<Record>& records)
{
    for (const LIEF::Section& section : binary.sections()) {
        if (section.name().empty())
            continue;

        records.push_back({
            "address",
            {
                section.name(),
                hex_value(section.offset()),
                hex_value(section.virtual_address())
            }
        });
    }
}

/*
 * @desc: Main executable inspection routine.
 */
static int execute(const Options& opt)
{
    if (!fs::exists(opt.file) || !fs::is_regular_file(opt.file)) {
        std::cerr << "exe: invalid path: " << opt.file << '\n';
        return 2;
    }

    std::ifstream input(opt.file, std::ios::binary);

    if (!input) {
        std::cerr << "exe: cannot open input file\n";
        return 2;
    }

    input.seekg(0, std::ios::end);
    std::streamoff end = input.tellg();

    if (end < 0) {
        std::cerr << "exe: cannot determine file size\n";
        return 2;
    }

    uint64_t file_size = static_cast<uint64_t>(end);
    input.seekg(0, std::ios::beg);

    std::vector<uint8_t> raw(
        static_cast<size_t>(file_size)
    );

    if (!raw.empty()) {
        input.read(
            reinterpret_cast<char*>(raw.data()),
            static_cast<std::streamsize>(raw.size())
        );

        if (!input) {
            std::cerr << "exe: failed to read input file\n";
            return 2;
        }
    }

    std::unique_ptr<LIEF::Binary> binary;

    try {
        binary = LIEF::Parser::parse(opt.file);
    }
    catch (const std::exception& e) {
        std::cerr << "exe: parser error: " << e.what() << '\n';
        return 1;
    }

    if (!binary ||
        (binary->format() != LIEF::Binary::FORMATS::ELF &&
         binary->format() != LIEF::Binary::FORMATS::PE)) {
        std::cerr << "exe: invalid or unsupported executable\n";
        return 1;
    }

    std::vector<Record> records;

    if (opt.info)
        collect_info(*binary, file_size, records);

    if (opt.sections)
        collect_sections(*binary, records);

    if (opt.addresses)
        collect_addresses(*binary, records);

    if (opt.mapping)
        collect_mappings(*binary, records);

    if (opt.strings)
        collect_strings(raw, records);

    if (opt.disassembly) {
        if (!collect_disassembly(*binary, records)) {
            std::cerr
                << "exe: cannot disassemble this architecture\n";
            return 1;
        }
    }

    std::ostream* output = &std::cout;
    std::ofstream output_file;

    if (!opt.output.empty()) {
        std::error_code ec;

        fs::path input_path = fs::weakly_canonical(opt.file, ec);
        ec.clear();

        fs::path output_path = fs::weakly_canonical(opt.output, ec);

        if (!ec && input_path == output_path) {
            std::cerr
                << "exe: output file must not overwrite the input file\n";
            return 4;
        }

        if (fs::exists(output_path) &&
            fs::is_directory(output_path)) {
            std::cerr << "exe: output path is a directory\n";
            return 2;
        }

        output_file.open(opt.output, std::ios::out | std::ios::trunc);

        if (!output_file) {
            std::cerr << "exe: cannot open output file\n";
            return 2;
        }

        output = &output_file;
    }

    if (opt.json)
        print_json(*output, records);
    else if (opt.pipe)
        print_pipe(*output, records);
    else
        print_human(*output, records);

    return 0;
}

int main(int argc, char** argv)
{
    Options opt;
    int status = 0;

    if (!parse_args(argc, argv, opt, status))
        return status;

    return execute(opt);
}
