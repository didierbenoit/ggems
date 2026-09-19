#include <algorithm>
#include <array>
#include <cstdint>
#include <format>
#include <span>
#include <utility>
#include <vector>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSIsotope.hh"
#include "GGEMS/materials/GGEMSIsotopeProfile.hh"
#include "GGEMS/materials/GGEMSIsotopicComposition.hh"

namespace ggems::core::materials {

namespace {

// =============================================================================
// =============================================================================

struct NaturalProfileRow {
  GGEMSIsotope isotope;
  long double atom_fraction;
};

// =============================================================================
// =============================================================================

[[nodiscard]] consteval auto
MakeNaturalRow(std::uint32_t atomic_number, std::uint32_t mass_number,
               std::uint32_t isomer_state, long double atom_fraction)
    -> NaturalProfileRow {
  return {
      .isotope = GGEMSIsotope{atomic_number, mass_number, isomer_state},
      .atom_fraction = atom_fraction,
  };
}

// =============================================================================
// =============================================================================

[[nodiscard]] consteval auto MakeReferenceRow(std::uint32_t atomic_number,
                                              std::uint32_t mass_number,
                                              std::uint32_t isomer_state)
    -> GGEMSIsotope {
  return GGEMSIsotope{atomic_number, mass_number, isomer_state};
}

// =============================================================================
// =============================================================================

constexpr auto k_nist41_natural_rows = std::array{
    MakeNaturalRow(1U, 1U, 0U, 0.999885L),
    MakeNaturalRow(1U, 2U, 0U, 0.000115L),
    MakeNaturalRow(2U, 3U, 0U, 0.00000134L),
    MakeNaturalRow(2U, 4U, 0U, 0.99999866L),
    MakeNaturalRow(3U, 6U, 0U, 0.0759L),
    MakeNaturalRow(3U, 7U, 0U, 0.9241L),
    MakeNaturalRow(4U, 9U, 0U, 1.0L),
    MakeNaturalRow(5U, 10U, 0U, 0.199L),
    MakeNaturalRow(5U, 11U, 0U, 0.801L),
    MakeNaturalRow(6U, 12U, 0U, 0.9893L),
    MakeNaturalRow(6U, 13U, 0U, 0.0107L),
    MakeNaturalRow(7U, 14U, 0U, 0.99636L),
    MakeNaturalRow(7U, 15U, 0U, 0.00364L),
    MakeNaturalRow(8U, 16U, 0U, 0.99757L),
    MakeNaturalRow(8U, 17U, 0U, 0.00038L),
    MakeNaturalRow(8U, 18U, 0U, 0.00205L),
    MakeNaturalRow(9U, 19U, 0U, 1.0L),
    MakeNaturalRow(10U, 20U, 0U, 0.9048L),
    MakeNaturalRow(10U, 21U, 0U, 0.0027L),
    MakeNaturalRow(10U, 22U, 0U, 0.0925L),
    MakeNaturalRow(11U, 23U, 0U, 1.0L),
    MakeNaturalRow(12U, 24U, 0U, 0.7899L),
    MakeNaturalRow(12U, 25U, 0U, 0.1000L),
    MakeNaturalRow(12U, 26U, 0U, 0.1101L),
    MakeNaturalRow(13U, 27U, 0U, 1.0L),
    MakeNaturalRow(14U, 28U, 0U, 0.92223L),
    MakeNaturalRow(14U, 29U, 0U, 0.04685L),
    MakeNaturalRow(14U, 30U, 0U, 0.03092L),
    MakeNaturalRow(15U, 31U, 0U, 1.0L),
    MakeNaturalRow(16U, 32U, 0U, 0.9499L),
    MakeNaturalRow(16U, 33U, 0U, 0.0075L),
    MakeNaturalRow(16U, 34U, 0U, 0.0425L),
    MakeNaturalRow(16U, 36U, 0U, 0.0001L),
    MakeNaturalRow(17U, 35U, 0U, 0.7576L),
    MakeNaturalRow(17U, 37U, 0U, 0.2424L),
    MakeNaturalRow(18U, 36U, 0U, 0.003336L),
    MakeNaturalRow(18U, 38U, 0U, 0.000629L),
    MakeNaturalRow(18U, 40U, 0U, 0.996035L),
    MakeNaturalRow(19U, 39U, 0U, 0.932581L),
    MakeNaturalRow(19U, 40U, 0U, 0.000117L),
    MakeNaturalRow(19U, 41U, 0U, 0.067302L),
    MakeNaturalRow(20U, 40U, 0U, 0.96941L),
    MakeNaturalRow(20U, 42U, 0U, 0.00647L),
    MakeNaturalRow(20U, 43U, 0U, 0.00135L),
    MakeNaturalRow(20U, 44U, 0U, 0.02086L),
    MakeNaturalRow(20U, 46U, 0U, 0.00004L),
    MakeNaturalRow(20U, 48U, 0U, 0.00187L),
    MakeNaturalRow(21U, 45U, 0U, 1.0L),
    MakeNaturalRow(22U, 46U, 0U, 0.0825L),
    MakeNaturalRow(22U, 47U, 0U, 0.0744L),
    MakeNaturalRow(22U, 48U, 0U, 0.7372L),
    MakeNaturalRow(22U, 49U, 0U, 0.0541L),
    MakeNaturalRow(22U, 50U, 0U, 0.0518L),
    MakeNaturalRow(23U, 50U, 0U, 0.00250L),
    MakeNaturalRow(23U, 51U, 0U, 0.99750L),
    MakeNaturalRow(24U, 50U, 0U, 0.04345L),
    MakeNaturalRow(24U, 52U, 0U, 0.83789L),
    MakeNaturalRow(24U, 53U, 0U, 0.09501L),
    MakeNaturalRow(24U, 54U, 0U, 0.02365L),
    MakeNaturalRow(25U, 55U, 0U, 1.0L),
    MakeNaturalRow(26U, 54U, 0U, 0.05845L),
    MakeNaturalRow(26U, 56U, 0U, 0.91754L),
    MakeNaturalRow(26U, 57U, 0U, 0.02119L),
    MakeNaturalRow(26U, 58U, 0U, 0.00282L),
    MakeNaturalRow(27U, 59U, 0U, 1.0L),
    MakeNaturalRow(28U, 58U, 0U, 0.68077L),
    MakeNaturalRow(28U, 60U, 0U, 0.26223L),
    MakeNaturalRow(28U, 61U, 0U, 0.011399L),
    MakeNaturalRow(28U, 62U, 0U, 0.036346L),
    MakeNaturalRow(28U, 64U, 0U, 0.009255L),
    MakeNaturalRow(29U, 63U, 0U, 0.6915L),
    MakeNaturalRow(29U, 65U, 0U, 0.3085L),
    MakeNaturalRow(30U, 64U, 0U, 0.4917L),
    MakeNaturalRow(30U, 66U, 0U, 0.2773L),
    MakeNaturalRow(30U, 67U, 0U, 0.0404L),
    MakeNaturalRow(30U, 68U, 0U, 0.1845L),
    MakeNaturalRow(30U, 70U, 0U, 0.0061L),
    MakeNaturalRow(31U, 69U, 0U, 0.60108L),
    MakeNaturalRow(31U, 71U, 0U, 0.39892L),
    MakeNaturalRow(32U, 70U, 0U, 0.2057L),
    MakeNaturalRow(32U, 72U, 0U, 0.2745L),
    MakeNaturalRow(32U, 73U, 0U, 0.0775L),
    MakeNaturalRow(32U, 74U, 0U, 0.3650L),
    MakeNaturalRow(32U, 76U, 0U, 0.0773L),
    MakeNaturalRow(33U, 75U, 0U, 1.0L),
    MakeNaturalRow(34U, 74U, 0U, 0.0089L),
    MakeNaturalRow(34U, 76U, 0U, 0.0937L),
    MakeNaturalRow(34U, 77U, 0U, 0.0763L),
    MakeNaturalRow(34U, 78U, 0U, 0.2377L),
    MakeNaturalRow(34U, 80U, 0U, 0.4961L),
    MakeNaturalRow(34U, 82U, 0U, 0.0873L),
    MakeNaturalRow(35U, 79U, 0U, 0.5069L),
    MakeNaturalRow(35U, 81U, 0U, 0.4931L),
    MakeNaturalRow(36U, 78U, 0U, 0.00355L),
    MakeNaturalRow(36U, 80U, 0U, 0.02286L),
    MakeNaturalRow(36U, 82U, 0U, 0.11593L),
    MakeNaturalRow(36U, 83U, 0U, 0.11500L),
    MakeNaturalRow(36U, 84U, 0U, 0.56987L),
    MakeNaturalRow(36U, 86U, 0U, 0.17279L),
    MakeNaturalRow(37U, 85U, 0U, 0.7217L),
    MakeNaturalRow(37U, 87U, 0U, 0.2783L),
    MakeNaturalRow(38U, 84U, 0U, 0.0056L),
    MakeNaturalRow(38U, 86U, 0U, 0.0986L),
    MakeNaturalRow(38U, 87U, 0U, 0.0700L),
    MakeNaturalRow(38U, 88U, 0U, 0.8258L),
    MakeNaturalRow(39U, 89U, 0U, 1.0L),
    MakeNaturalRow(40U, 90U, 0U, 0.5145L),
    MakeNaturalRow(40U, 91U, 0U, 0.1122L),
    MakeNaturalRow(40U, 92U, 0U, 0.1715L),
    MakeNaturalRow(40U, 94U, 0U, 0.1738L),
    MakeNaturalRow(40U, 96U, 0U, 0.0280L),
    MakeNaturalRow(41U, 93U, 0U, 1.0L),
    MakeNaturalRow(42U, 92U, 0U, 0.1453L),
    MakeNaturalRow(42U, 94U, 0U, 0.0915L),
    MakeNaturalRow(42U, 95U, 0U, 0.1584L),
    MakeNaturalRow(42U, 96U, 0U, 0.1667L),
    MakeNaturalRow(42U, 97U, 0U, 0.0960L),
    MakeNaturalRow(42U, 98U, 0U, 0.2439L),
    MakeNaturalRow(42U, 100U, 0U, 0.0982L),
    MakeNaturalRow(44U, 96U, 0U, 0.0554L),
    MakeNaturalRow(44U, 98U, 0U, 0.0187L),
    MakeNaturalRow(44U, 99U, 0U, 0.1276L),
    MakeNaturalRow(44U, 100U, 0U, 0.1260L),
    MakeNaturalRow(44U, 101U, 0U, 0.1706L),
    MakeNaturalRow(44U, 102U, 0U, 0.3155L),
    MakeNaturalRow(44U, 104U, 0U, 0.1862L),
    MakeNaturalRow(45U, 103U, 0U, 1.0L),
    MakeNaturalRow(46U, 102U, 0U, 0.0102L),
    MakeNaturalRow(46U, 104U, 0U, 0.1114L),
    MakeNaturalRow(46U, 105U, 0U, 0.2233L),
    MakeNaturalRow(46U, 106U, 0U, 0.2733L),
    MakeNaturalRow(46U, 108U, 0U, 0.2646L),
    MakeNaturalRow(46U, 110U, 0U, 0.1172L),
    MakeNaturalRow(47U, 107U, 0U, 0.51839L),
    MakeNaturalRow(47U, 109U, 0U, 0.48161L),
    MakeNaturalRow(48U, 106U, 0U, 0.0125L),
    MakeNaturalRow(48U, 108U, 0U, 0.0089L),
    MakeNaturalRow(48U, 110U, 0U, 0.1249L),
    MakeNaturalRow(48U, 111U, 0U, 0.1280L),
    MakeNaturalRow(48U, 112U, 0U, 0.2413L),
    MakeNaturalRow(48U, 113U, 0U, 0.1222L),
    MakeNaturalRow(48U, 114U, 0U, 0.2873L),
    MakeNaturalRow(48U, 116U, 0U, 0.0749L),
    MakeNaturalRow(49U, 113U, 0U, 0.0429L),
    MakeNaturalRow(49U, 115U, 0U, 0.9571L),
    MakeNaturalRow(50U, 112U, 0U, 0.0097L),
    MakeNaturalRow(50U, 114U, 0U, 0.0066L),
    MakeNaturalRow(50U, 115U, 0U, 0.0034L),
    MakeNaturalRow(50U, 116U, 0U, 0.1454L),
    MakeNaturalRow(50U, 117U, 0U, 0.0768L),
    MakeNaturalRow(50U, 118U, 0U, 0.2422L),
    MakeNaturalRow(50U, 119U, 0U, 0.0859L),
    MakeNaturalRow(50U, 120U, 0U, 0.3258L),
    MakeNaturalRow(50U, 122U, 0U, 0.0463L),
    MakeNaturalRow(50U, 124U, 0U, 0.0579L),
    MakeNaturalRow(51U, 121U, 0U, 0.5721L),
    MakeNaturalRow(51U, 123U, 0U, 0.4279L),
    MakeNaturalRow(52U, 120U, 0U, 0.0009L),
    MakeNaturalRow(52U, 122U, 0U, 0.0255L),
    MakeNaturalRow(52U, 123U, 0U, 0.0089L),
    MakeNaturalRow(52U, 124U, 0U, 0.0474L),
    MakeNaturalRow(52U, 125U, 0U, 0.0707L),
    MakeNaturalRow(52U, 126U, 0U, 0.1884L),
    MakeNaturalRow(52U, 128U, 0U, 0.3174L),
    MakeNaturalRow(52U, 130U, 0U, 0.3408L),
    MakeNaturalRow(53U, 127U, 0U, 1.0L),
    MakeNaturalRow(54U, 124U, 0U, 0.000952L),
    MakeNaturalRow(54U, 126U, 0U, 0.000890L),
    MakeNaturalRow(54U, 128U, 0U, 0.019102L),
    MakeNaturalRow(54U, 129U, 0U, 0.264006L),
    MakeNaturalRow(54U, 130U, 0U, 0.040710L),
    MakeNaturalRow(54U, 131U, 0U, 0.212324L),
    MakeNaturalRow(54U, 132U, 0U, 0.269086L),
    MakeNaturalRow(54U, 134U, 0U, 0.104357L),
    MakeNaturalRow(54U, 136U, 0U, 0.088573L),
    MakeNaturalRow(55U, 133U, 0U, 1.0L),
    MakeNaturalRow(56U, 130U, 0U, 0.00106L),
    MakeNaturalRow(56U, 132U, 0U, 0.00101L),
    MakeNaturalRow(56U, 134U, 0U, 0.02417L),
    MakeNaturalRow(56U, 135U, 0U, 0.06592L),
    MakeNaturalRow(56U, 136U, 0U, 0.07854L),
    MakeNaturalRow(56U, 137U, 0U, 0.11232L),
    MakeNaturalRow(56U, 138U, 0U, 0.71698L),
    MakeNaturalRow(57U, 138U, 0U, 0.0008881L),
    MakeNaturalRow(57U, 139U, 0U, 0.9991119L),
    MakeNaturalRow(58U, 136U, 0U, 0.00185L),
    MakeNaturalRow(58U, 138U, 0U, 0.00251L),
    MakeNaturalRow(58U, 140U, 0U, 0.88450L),
    MakeNaturalRow(58U, 142U, 0U, 0.11114L),
    MakeNaturalRow(59U, 141U, 0U, 1.0L),
    MakeNaturalRow(60U, 142U, 0U, 0.27152L),
    MakeNaturalRow(60U, 143U, 0U, 0.12174L),
    MakeNaturalRow(60U, 144U, 0U, 0.23798L),
    MakeNaturalRow(60U, 145U, 0U, 0.08293L),
    MakeNaturalRow(60U, 146U, 0U, 0.17189L),
    MakeNaturalRow(60U, 148U, 0U, 0.05756L),
    MakeNaturalRow(60U, 150U, 0U, 0.05638L),
    MakeNaturalRow(62U, 144U, 0U, 0.0307L),
    MakeNaturalRow(62U, 147U, 0U, 0.1499L),
    MakeNaturalRow(62U, 148U, 0U, 0.1124L),
    MakeNaturalRow(62U, 149U, 0U, 0.1382L),
    MakeNaturalRow(62U, 150U, 0U, 0.0738L),
    MakeNaturalRow(62U, 152U, 0U, 0.2675L),
    MakeNaturalRow(62U, 154U, 0U, 0.2275L),
    MakeNaturalRow(63U, 151U, 0U, 0.4781L),
    MakeNaturalRow(63U, 153U, 0U, 0.5219L),
    MakeNaturalRow(64U, 152U, 0U, 0.0020L),
    MakeNaturalRow(64U, 154U, 0U, 0.0218L),
    MakeNaturalRow(64U, 155U, 0U, 0.1480L),
    MakeNaturalRow(64U, 156U, 0U, 0.2047L),
    MakeNaturalRow(64U, 157U, 0U, 0.1565L),
    MakeNaturalRow(64U, 158U, 0U, 0.2484L),
    MakeNaturalRow(64U, 160U, 0U, 0.2186L),
    MakeNaturalRow(65U, 159U, 0U, 1.0L),
    MakeNaturalRow(66U, 156U, 0U, 0.00056L),
    MakeNaturalRow(66U, 158U, 0U, 0.00095L),
    MakeNaturalRow(66U, 160U, 0U, 0.02329L),
    MakeNaturalRow(66U, 161U, 0U, 0.18889L),
    MakeNaturalRow(66U, 162U, 0U, 0.25475L),
    MakeNaturalRow(66U, 163U, 0U, 0.24896L),
    MakeNaturalRow(66U, 164U, 0U, 0.28260L),
    MakeNaturalRow(67U, 165U, 0U, 1.0L),
    MakeNaturalRow(68U, 162U, 0U, 0.00139L),
    MakeNaturalRow(68U, 164U, 0U, 0.01601L),
    MakeNaturalRow(68U, 166U, 0U, 0.33503L),
    MakeNaturalRow(68U, 167U, 0U, 0.22869L),
    MakeNaturalRow(68U, 168U, 0U, 0.26978L),
    MakeNaturalRow(68U, 170U, 0U, 0.14910L),
    MakeNaturalRow(69U, 169U, 0U, 1.0L),
    MakeNaturalRow(70U, 168U, 0U, 0.00123L),
    MakeNaturalRow(70U, 170U, 0U, 0.02982L),
    MakeNaturalRow(70U, 171U, 0U, 0.1409L),
    MakeNaturalRow(70U, 172U, 0U, 0.2168L),
    MakeNaturalRow(70U, 173U, 0U, 0.16103L),
    MakeNaturalRow(70U, 174U, 0U, 0.32026L),
    MakeNaturalRow(70U, 176U, 0U, 0.12996L),
    MakeNaturalRow(71U, 175U, 0U, 0.97401L),
    MakeNaturalRow(71U, 176U, 0U, 0.02599L),
    MakeNaturalRow(72U, 174U, 0U, 0.0016L),
    MakeNaturalRow(72U, 176U, 0U, 0.0526L),
    MakeNaturalRow(72U, 177U, 0U, 0.1860L),
    MakeNaturalRow(72U, 178U, 0U, 0.2728L),
    MakeNaturalRow(72U, 179U, 0U, 0.1362L),
    MakeNaturalRow(72U, 180U, 0U, 0.3508L),
    MakeNaturalRow(73U, 180U, 1U, 0.0001201L),
    MakeNaturalRow(73U, 181U, 0U, 0.9998799L),
    MakeNaturalRow(74U, 180U, 0U, 0.0012L),
    MakeNaturalRow(74U, 182U, 0U, 0.2650L),
    MakeNaturalRow(74U, 183U, 0U, 0.1431L),
    MakeNaturalRow(74U, 184U, 0U, 0.3064L),
    MakeNaturalRow(74U, 186U, 0U, 0.2843L),
    MakeNaturalRow(75U, 185U, 0U, 0.3740L),
    MakeNaturalRow(75U, 187U, 0U, 0.6260L),
    MakeNaturalRow(76U, 184U, 0U, 0.0002L),
    MakeNaturalRow(76U, 186U, 0U, 0.0159L),
    MakeNaturalRow(76U, 187U, 0U, 0.0196L),
    MakeNaturalRow(76U, 188U, 0U, 0.1324L),
    MakeNaturalRow(76U, 189U, 0U, 0.1615L),
    MakeNaturalRow(76U, 190U, 0U, 0.2626L),
    MakeNaturalRow(76U, 192U, 0U, 0.4078L),
    MakeNaturalRow(77U, 191U, 0U, 0.373L),
    MakeNaturalRow(77U, 193U, 0U, 0.627L),
    MakeNaturalRow(78U, 190U, 0U, 0.00012L),
    MakeNaturalRow(78U, 192U, 0U, 0.00782L),
    MakeNaturalRow(78U, 194U, 0U, 0.3286L),
    MakeNaturalRow(78U, 195U, 0U, 0.3378L),
    MakeNaturalRow(78U, 196U, 0U, 0.2521L),
    MakeNaturalRow(78U, 198U, 0U, 0.07356L),
    MakeNaturalRow(79U, 197U, 0U, 1.0L),
    MakeNaturalRow(80U, 196U, 0U, 0.0015L),
    MakeNaturalRow(80U, 198U, 0U, 0.0997L),
    MakeNaturalRow(80U, 199U, 0U, 0.1687L),
    MakeNaturalRow(80U, 200U, 0U, 0.2310L),
    MakeNaturalRow(80U, 201U, 0U, 0.1318L),
    MakeNaturalRow(80U, 202U, 0U, 0.2986L),
    MakeNaturalRow(80U, 204U, 0U, 0.0687L),
    MakeNaturalRow(81U, 203U, 0U, 0.2952L),
    MakeNaturalRow(81U, 205U, 0U, 0.7048L),
    MakeNaturalRow(82U, 204U, 0U, 0.014L),
    MakeNaturalRow(82U, 206U, 0U, 0.241L),
    MakeNaturalRow(82U, 207U, 0U, 0.221L),
    MakeNaturalRow(82U, 208U, 0U, 0.524L),
    MakeNaturalRow(83U, 209U, 0U, 1.0L),
    MakeNaturalRow(90U, 232U, 0U, 1.0L),
    MakeNaturalRow(91U, 231U, 0U, 1.0L),
    MakeNaturalRow(92U, 234U, 0U, 0.000054L),
    MakeNaturalRow(92U, 235U, 0U, 0.007204L),
    MakeNaturalRow(92U, 238U, 0U, 0.992742L),
};

// =============================================================================
// =============================================================================

constexpr auto k_legacy_reference_isotopes = std::array{
    MakeReferenceRow(43U, 97U, 0U),  MakeReferenceRow(61U, 145U, 0U),
    MakeReferenceRow(84U, 209U, 0U), MakeReferenceRow(85U, 210U, 0U),
    MakeReferenceRow(86U, 222U, 0U), MakeReferenceRow(87U, 223U, 0U),
    MakeReferenceRow(88U, 226U, 0U), MakeReferenceRow(89U, 227U, 0U),
};

// =============================================================================
// =============================================================================

[[nodiscard]] auto FindNaturalRows(std::uint32_t atomic_number) noexcept
    -> std::span<NaturalProfileRow const> {
  auto const rows = std::ranges::equal_range(
      k_nist41_natural_rows, atomic_number, {},
      [](NaturalProfileRow const &row) noexcept -> std::uint32_t {
        return row.isotope.GetAtomicNumber();
      });

  return {rows.begin(), rows.end()};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto FindReferenceIsotope(std::uint32_t atomic_number) noexcept
    -> GGEMSIsotope const * {
  auto const found =
      std::ranges::find(k_legacy_reference_isotopes, atomic_number,
                        &GGEMSIsotope::GetAtomicNumber);

  return found == k_legacy_reference_isotopes.end() ? nullptr : &*found;
}

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto HasIsotopeProfile(GGEMSIsotopeProfile profile,
                                     std::uint32_t atomic_number) noexcept
    -> bool {
  switch (profile) {
  case GGEMSIsotopeProfile::Nist41Natural:
    return !FindNaturalRows(atomic_number).empty();
  case GGEMSIsotopeProfile::LegacyReferenceIsotope:
    return FindReferenceIsotope(atomic_number) != nullptr;
  }

  return false;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto ResolveIsotopeProfile(GGEMSIsotopeProfile profile,
                                         std::uint32_t atomic_number)
    -> GGEMSIsotopicComposition {
  std::vector<GGEMSIsotopeFraction> fractions;

  switch (profile) {
  case GGEMSIsotopeProfile::Nist41Natural:
    for (auto const &row : FindNaturalRows(atomic_number)) {
      fractions.push_back({
          .isotope = row.isotope,
          .fraction = row.atom_fraction,
      });
    }
    break;
  case GGEMSIsotopeProfile::LegacyReferenceIsotope:
    if (auto const *isotope = FindReferenceIsotope(atomic_number);
        isotope != nullptr) {
      fractions.push_back({.isotope = *isotope, .fraction = 1.0L});
    }
    break;
  }

  if (fractions.empty()) {
    throw GGEMSRecoverable{
        std::format("Isotope has no composition for Z={}.", atomic_number)};
  }

  return GGEMSIsotopicComposition{GGEMSFractionBasis::AtomFraction,
                                  std::move(fractions)};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
SelectLegacyElementalIsotopeProfile(std::uint32_t atomic_number)
    -> GGEMSIsotopeProfile {
  if (HasIsotopeProfile(GGEMSIsotopeProfile::Nist41Natural, atomic_number)) {
    return GGEMSIsotopeProfile::Nist41Natural;
  }

  if (HasIsotopeProfile(GGEMSIsotopeProfile::LegacyReferenceIsotope,
                        atomic_number)) {
    return GGEMSIsotopeProfile::LegacyReferenceIsotope;
  }

  throw GGEMSRecoverable{std::format(
      "No qualified isotope composition profile for Z={}.", atomic_number)};
}

} // namespace ggems::core::materials
