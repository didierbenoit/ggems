#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <span>
#include <string_view>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSElement.hh"
#include "GGEMS/materials/GGEMSElementCatalog.hh"

namespace ggems::core::materials {
namespace {

constexpr std::size_t k_element_count{118U};

// =============================================================================
// =============================================================================

[[nodiscard]] consteval auto
MakeElementRow(std::uint32_t atomic_number, std::string_view symbol,
               std::string_view name, long double molar_mass) -> GGEMSElement {
  return GGEMSElement{atomic_number, symbol, name, molar_mass};
}

// =============================================================================
// =============================================================================

constexpr auto k_elements = std::array{
    MakeElementRow(1U, "H", "hydrogen", 1.0080L),
    MakeElementRow(2U, "He", "helium", 4.002602L),
    // CIAAW interval [6.938, 6.997]; using abridged reference 6.94.
    MakeElementRow(3U, "Li", "lithium", 6.94L),
    MakeElementRow(4U, "Be", "beryllium", 9.0121831L),
    // CIAAW interval [10.806, 10.821]; using abridged reference 10.81.
    MakeElementRow(5U, "B", "boron", 10.81L),
    // CIAAW interval [12.0096, 12.0116]; using abridged reference 12.011.
    MakeElementRow(6U, "C", "carbon", 12.011L),
    // CIAAW interval [14.00643, 14.00728]; using abridged reference 14.007.
    MakeElementRow(7U, "N", "nitrogen", 14.007L),
    // CIAAW interval [15.99903, 15.99977]; using abridged reference 15.999.
    MakeElementRow(8U, "O", "oxygen", 15.999L),
    MakeElementRow(9U, "F", "fluorine", 18.998403162L),
    MakeElementRow(10U, "Ne", "neon", 20.1797L),
    MakeElementRow(11U, "Na", "sodium", 22.98976928L),
    // CIAAW interval [24.304, 24.307]; using abridged reference 24.305.
    MakeElementRow(12U, "Mg", "magnesium", 24.305L),
    MakeElementRow(13U, "Al", "aluminium", 26.9815384L),
    // CIAAW interval [28.084, 28.086]; using abridged reference 28.085.
    MakeElementRow(14U, "Si", "silicon", 28.085L),
    MakeElementRow(15U, "P", "phosphorus", 30.973761998L),
    // CIAAW interval [32.059, 32.076]; using abridged reference 32.06.
    MakeElementRow(16U, "S", "sulfur", 32.06L),
    // CIAAW interval [35.446, 35.457]; using abridged reference 35.45.
    MakeElementRow(17U, "Cl", "chlorine", 35.45L),
    // CIAAW interval [39.792, 39.963]; using abridged reference 39.95.
    MakeElementRow(18U, "Ar", "argon", 39.95L),
    MakeElementRow(19U, "K", "potassium", 39.0983L),
    MakeElementRow(20U, "Ca", "calcium", 40.078L),
    MakeElementRow(21U, "Sc", "scandium", 44.955907L),
    MakeElementRow(22U, "Ti", "titanium", 47.867L),
    MakeElementRow(23U, "V", "vanadium", 50.9415L),
    MakeElementRow(24U, "Cr", "chromium", 51.9961L),
    MakeElementRow(25U, "Mn", "manganese", 54.938043L),
    MakeElementRow(26U, "Fe", "iron", 55.845L),
    MakeElementRow(27U, "Co", "cobalt", 58.933194L),
    MakeElementRow(28U, "Ni", "nickel", 58.6934L),
    MakeElementRow(29U, "Cu", "copper", 63.546L),
    MakeElementRow(30U, "Zn", "zinc", 65.38L),
    MakeElementRow(31U, "Ga", "gallium", 69.723L),
    MakeElementRow(32U, "Ge", "germanium", 72.630L),
    MakeElementRow(33U, "As", "arsenic", 74.921595L),
    MakeElementRow(34U, "Se", "selenium", 78.971L),
    // CIAAW interval [79.901, 79.907]; using abridged reference 79.904.
    MakeElementRow(35U, "Br", "bromine", 79.904L),
    MakeElementRow(36U, "Kr", "krypton", 83.798L),
    MakeElementRow(37U, "Rb", "rubidium", 85.4678L),
    MakeElementRow(38U, "Sr", "strontium", 87.62L),
    MakeElementRow(39U, "Y", "yttrium", 88.905838L),
    // CIAAW 2024 revision: 91.222(3), replacing 91.224(2).
    MakeElementRow(40U, "Zr", "zirconium", 91.222L),
    MakeElementRow(41U, "Nb", "niobium", 92.90637L),
    MakeElementRow(42U, "Mo", "molybdenum", 95.95L),
    // No CIAAW standard atomic weight; reference isotope Tc-97 (AME2020).
    // Multiple candidate isotopes; current IUPAC table selects Tc-97.
    MakeElementRow(43U, "Tc", "technetium", 96.906360720L),
    MakeElementRow(44U, "Ru", "ruthenium", 101.07L),
    MakeElementRow(45U, "Rh", "rhodium", 102.90549L),
    MakeElementRow(46U, "Pd", "palladium", 106.42L),
    MakeElementRow(47U, "Ag", "silver", 107.8682L),
    MakeElementRow(48U, "Cd", "cadmium", 112.414L),
    MakeElementRow(49U, "In", "indium", 114.818L),
    MakeElementRow(50U, "Sn", "tin", 118.710L),
    MakeElementRow(51U, "Sb", "antimony", 121.760L),
    MakeElementRow(52U, "Te", "tellurium", 127.60L),
    MakeElementRow(53U, "I", "iodine", 126.90447L),
    MakeElementRow(54U, "Xe", "xenon", 131.293L),
    MakeElementRow(55U, "Cs", "caesium", 132.90545196L),
    MakeElementRow(56U, "Ba", "barium", 137.327L),
    MakeElementRow(57U, "La", "lanthanum", 138.90547L),
    MakeElementRow(58U, "Ce", "cerium", 140.116L),
    MakeElementRow(59U, "Pr", "praseodymium", 140.90766L),
    MakeElementRow(60U, "Nd", "neodymium", 144.242L),
    // No CIAAW standard atomic weight; reference isotope Pm-145 (AME2020).
    MakeElementRow(61U, "Pm", "promethium", 144.912755748L),
    MakeElementRow(62U, "Sm", "samarium", 150.36L),
    MakeElementRow(63U, "Eu", "europium", 151.964L),
    // CIAAW 2024 revision: 157.249(2), replacing 157.25(3).
    MakeElementRow(64U, "Gd", "gadolinium", 157.249L),
    MakeElementRow(65U, "Tb", "terbium", 158.925354L),
    MakeElementRow(66U, "Dy", "dysprosium", 162.500L),
    MakeElementRow(67U, "Ho", "holmium", 164.930329L),
    MakeElementRow(68U, "Er", "erbium", 167.259L),
    MakeElementRow(69U, "Tm", "thulium", 168.934219L),
    MakeElementRow(70U, "Yb", "ytterbium", 173.045L),
    // CIAAW 2024 revision: 174.966 69(5), replacing 174.9668(1).
    MakeElementRow(71U, "Lu", "lutetium", 174.96669L),
    MakeElementRow(72U, "Hf", "hafnium", 178.486L),
    MakeElementRow(73U, "Ta", "tantalum", 180.94788L),
    MakeElementRow(74U, "W", "tungsten", 183.84L),
    MakeElementRow(75U, "Re", "rhenium", 186.207L),
    MakeElementRow(76U, "Os", "osmium", 190.23L),
    MakeElementRow(77U, "Ir", "iridium", 192.217L),
    MakeElementRow(78U, "Pt", "platinum", 195.084L),
    MakeElementRow(79U, "Au", "gold", 196.966570L),
    MakeElementRow(80U, "Hg", "mercury", 200.592L),
    // CIAAW interval [204.382, 204.385]; using abridged reference 204.38.
    MakeElementRow(81U, "Tl", "thallium", 204.38L),
    // CIAAW interval [206.14, 207.94]; using abridged reference 207.2.
    MakeElementRow(82U, "Pb", "lead", 207.2L),
    MakeElementRow(83U, "Bi", "bismuth", 208.98040L),
    // No CIAAW standard atomic weight; reference isotope Po-209 (AME2020).
    MakeElementRow(84U, "Po", "polonium", 208.982430361L),
    // No CIAAW standard atomic weight; reference isotope At-210 (AME2020).
    MakeElementRow(85U, "At", "astatine", 209.987147423L),
    // No CIAAW standard atomic weight; reference isotope Rn-222 (AME2020).
    MakeElementRow(86U, "Rn", "radon", 222.017576017L),
    // No CIAAW standard atomic weight; reference isotope Fr-223 (AME2020).
    MakeElementRow(87U, "Fr", "francium", 223.019734241L),
    // No CIAAW standard atomic weight; reference isotope Ra-226 (AME2020).
    MakeElementRow(88U, "Ra", "radium", 226.025408186L),
    // No CIAAW standard atomic weight; reference isotope Ac-227 (AME2020).
    MakeElementRow(89U, "Ac", "actinium", 227.027750594L),
    MakeElementRow(90U, "Th", "thorium", 232.0377L),
    MakeElementRow(91U, "Pa", "protactinium", 231.03588L),
    MakeElementRow(92U, "U", "uranium", 238.02891L),
    // No CIAAW standard atomic weight; reference isotope Np-237 (AME2020).
    MakeElementRow(93U, "Np", "neptunium", 237.048171640L),
    // No CIAAW standard atomic weight; reference isotope Pu-244 (AME2020).
    MakeElementRow(94U, "Pu", "plutonium", 244.064204401L),
    // No CIAAW standard atomic weight; reference isotope Am-243 (AME2020).
    MakeElementRow(95U, "Am", "americium", 243.061379889L),
    // No CIAAW standard atomic weight; reference isotope Cm-247 (AME2020).
    MakeElementRow(96U, "Cm", "curium", 247.070352678L),
    // No CIAAW standard atomic weight; reference isotope Bk-247 (AME2020).
    MakeElementRow(97U, "Bk", "berkelium", 247.070305889L),
    // No CIAAW standard atomic weight; reference isotope Cf-251 (AME2020).
    MakeElementRow(98U, "Cf", "californium", 251.079587171L),
    // No CIAAW standard atomic weight; reference isotope Es-252 (AME2020).
    MakeElementRow(99U, "Es", "einsteinium", 252.082979173L),
    // No CIAAW standard atomic weight; reference isotope Fm-257 (AME2020).
    MakeElementRow(100U, "Fm", "fermium", 257.095105419L),
    // No CIAAW standard atomic weight; reference isotope Md-258 (AME2020).
    MakeElementRow(101U, "Md", "mendelevium", 258.098433634L),
    // No CIAAW standard atomic weight; reference isotope No-259 (AME2020).
    MakeElementRow(102U, "No", "nobelium", 259.100998364L),
    // No CIAAW standard atomic weight; reference isotope Lr-262 (AME2020).
    // AME2020 atomic mass is estimated (#).
    // Multiple candidate isotopes; current IUPAC table selects Lr-262.
    MakeElementRow(103U, "Lr", "lawrencium", 262.109615L),
    // No CIAAW standard atomic weight; reference isotope Rf-267 (AME2020).
    // AME2020 atomic mass is estimated (#).
    MakeElementRow(104U, "Rf", "rutherfordium", 267.121787L),
    // No CIAAW standard atomic weight; reference isotope Db-268 (AME2020).
    // AME2020 atomic mass is estimated (#).
    MakeElementRow(105U, "Db", "dubnium", 268.125669L),
    // No CIAAW standard atomic weight; reference isotope Sg-269 (AME2020).
    // AME2020 atomic mass is estimated (#).
    // Multiple candidate isotopes; current IUPAC table selects Sg-269.
    MakeElementRow(106U, "Sg", "seaborgium", 269.128495L),
    // No CIAAW standard atomic weight; reference isotope Bh-270 (AME2020).
    // AME2020 atomic mass is estimated (#).
    // Multiple candidate isotopes; current IUPAC table selects Bh-270.
    MakeElementRow(107U, "Bh", "bohrium", 270.133366L),
    // No CIAAW standard atomic weight; reference isotope Hs-269 (AME2020).
    // AME2020 atomic mass is estimated (#).
    // Multiple candidate isotopes; current IUPAC table selects Hs-269.
    MakeElementRow(108U, "Hs", "hassium", 269.133649L),
    // No CIAAW standard atomic weight; reference isotope Mt-277 (AME2020).
    // AME2020 atomic mass is estimated (#).
    // Multiple candidate isotopes; current IUPAC table selects Mt-277.
    MakeElementRow(109U, "Mt", "meitnerium", 277.153525L),
    // No CIAAW standard atomic weight; reference isotope Ds-281 (AME2020).
    // AME2020 atomic mass is estimated (#).
    MakeElementRow(110U, "Ds", "darmstadtium", 281.164545L),
    // No CIAAW standard atomic weight; reference isotope Rg-282 (AME2020).
    // AME2020 atomic mass is estimated (#).
    MakeElementRow(111U, "Rg", "roentgenium", 282.169343L),
    // No CIAAW standard atomic weight; reference isotope Cn-285 (AME2020).
    // AME2020 atomic mass is estimated (#).
    MakeElementRow(112U, "Cn", "copernicium", 285.177227L),
    // No CIAAW standard atomic weight; reference isotope Nh-286 (AME2020).
    // AME2020 atomic mass is estimated (#).
    // Multiple candidate isotopes; current IUPAC table selects Nh-286.
    MakeElementRow(113U, "Nh", "nihonium", 286.182456L),
    // No CIAAW standard atomic weight; reference isotope Fl-290 (AME2020).
    // AME2020 atomic mass is estimated (#).
    // Multiple candidate isotopes; current IUPAC table selects Fl-290.
    MakeElementRow(114U, "Fl", "flerovium", 290.191875L),
    // No CIAAW standard atomic weight; reference isotope Mc-290 (AME2020).
    // AME2020 atomic mass is estimated (#).
    // Multiple candidate isotopes; current IUPAC table selects Mc-290.
    MakeElementRow(115U, "Mc", "moscovium", 290.196235L),
    // No CIAAW standard atomic weight; reference isotope Lv-293 (AME2020).
    // AME2020 atomic mass is estimated (#).
    // Multiple candidate isotopes; current IUPAC table selects Lv-293.
    MakeElementRow(116U, "Lv", "livermorium", 293.204583L),
    // No CIAAW standard atomic weight; reference isotope Ts-294 (AME2020).
    // AME2020 atomic mass is estimated (#).
    MakeElementRow(117U, "Ts", "tennessine", 294.210840L),
    // No CIAAW standard atomic weight; reference isotope Og-294 (AME2020).
    // AME2020 atomic mass is estimated (#).
    MakeElementRow(118U, "Og", "oganesson", 294.213979L)};

// =============================================================================
// =============================================================================

[[nodiscard]] consteval auto ValidateAtomicNumbers() -> bool {
  for (std::size_t i = 0U; i < k_elements.size(); ++i) {
    if (k_elements[i].GetAtomicNumber() != i + 1U) {
      return false;
    }
  }

  return true;
}

static_assert(k_elements.size() == k_element_count);
static_assert(ValidateAtomicNumbers());

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto GetElements() noexcept -> std::span<GGEMSElement const> {
  return k_elements;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
FindElementByAtomicNumber(std::uint32_t atomic_number) noexcept
    -> GGEMSElement const * {
  if (atomic_number < 1U || atomic_number > k_elements.size()) {
    return nullptr;
  }

  return &k_elements[atomic_number - 1U];
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto FindElementBySymbol(std::string_view symbol) noexcept
    -> GGEMSElement const * {
  for (auto const &element : k_elements) {
    if (element.GetSymbol() == symbol) {
      return &element;
    }
  }

  return nullptr;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto FindElementByName(std::string_view name) noexcept
    -> GGEMSElement const * {
  for (auto const &element : k_elements) {
    if (element.GetName() == name) {
      return &element;
    }
  }

  return nullptr;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto RequireElementByAtomicNumber(std::uint32_t atomic_number)
    -> GGEMSElement const & {
  auto const *element = FindElementByAtomicNumber(atomic_number);

  if (element == nullptr) {
    throw GGEMSRecoverable{
        std::format("Unknown element atomic number {}.", atomic_number)};
  }

  return *element;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto RequireElementBySymbol(std::string_view symbol)
    -> GGEMSElement const & {
  auto const *element = FindElementBySymbol(symbol);

  if (element == nullptr) {
    throw GGEMSRecoverable{std::format("Unknown element symbol '{}'.", symbol)};
  }

  return *element;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto RequireElementByName(std::string_view name)
    -> GGEMSElement const & {
  auto const *element = FindElementByName(name);

  if (element == nullptr) {
    throw GGEMSRecoverable{std::format("Unknown element name '{}'.", name)};
  }

  return *element;
}

} // namespace ggems::core::materials
