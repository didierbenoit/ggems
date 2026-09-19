#include <array>
#include <cstdint>
#include <utility>
#include <vector>

#include "GGEMS/materials/GGEMSIsotope.hh"
#include "GGEMS/materials/GGEMSIsotopeMassAuthority.hh"
#include "GGEMS/materials/GGEMSResolvedIsotopeTable.hh"

namespace ggems::core::materials {

namespace {

// =============================================================================
// =============================================================================

struct IsotopeMassRow {
  GGEMSIsotope isotope;
  long double ground_state_relative_atomic_mass;
  long double excitation_energy_kilo_electron_volts;
};

// =============================================================================
// =============================================================================

[[nodiscard]] consteval auto MakeIsotopeMassRow(
    std::uint32_t atomic_number, std::uint32_t mass_number,
    std::uint32_t isomer_state, long double ground_state_relative_atomic_mass,
    long double excitation_energy_kilo_electron_volts) -> IsotopeMassRow {
  return {
      .isotope = GGEMSIsotope{atomic_number, mass_number, isomer_state},
      .ground_state_relative_atomic_mass = ground_state_relative_atomic_mass,
      .excitation_energy_kilo_electron_volts =
          excitation_energy_kilo_electron_volts,
  };
}

// =============================================================================
// =============================================================================

constexpr long double k_atomic_mass_unit_kilo_electron_volts{931494.10372L};
constexpr long double k_molar_mass_constant_grams_per_mole{1.00000000105L};

constexpr auto k_isotope_masses = std::array{
    MakeIsotopeMassRow(1U, 1U, 0U, 1.007825031898L, 0.0L),
    MakeIsotopeMassRow(1U, 2U, 0U, 2.014101777844L, 0.0L),
    MakeIsotopeMassRow(2U, 3U, 0U, 3.01602932197L, 0.0L),
    MakeIsotopeMassRow(2U, 4U, 0U, 4.00260325413L, 0.0L),
    MakeIsotopeMassRow(3U, 6U, 0U, 6.01512288742L, 0.0L),
    MakeIsotopeMassRow(3U, 7U, 0U, 7.01600343426L, 0.0L),
    MakeIsotopeMassRow(4U, 9U, 0U, 9.012183062L, 0.0L),
    MakeIsotopeMassRow(5U, 10U, 0U, 10.012936862L, 0.0L),
    MakeIsotopeMassRow(5U, 11U, 0U, 11.009305166L, 0.0L),
    MakeIsotopeMassRow(6U, 12U, 0U, 12.0L, 0.0L),
    MakeIsotopeMassRow(6U, 13U, 0U, 13.00335483534L, 0.0L),
    MakeIsotopeMassRow(7U, 14U, 0U, 14.00307400425L, 0.0L),
    MakeIsotopeMassRow(7U, 15U, 0U, 15.00010889827L, 0.0L),
    MakeIsotopeMassRow(8U, 16U, 0U, 15.99491461926L, 0.0L),
    MakeIsotopeMassRow(8U, 17U, 0U, 16.99913175595L, 0.0L),
    MakeIsotopeMassRow(8U, 18U, 0U, 17.99915961214L, 0.0L),
    MakeIsotopeMassRow(9U, 19U, 0U, 18.99840316207L, 0.0L),
    MakeIsotopeMassRow(10U, 20U, 0U, 19.99244017525L, 0.0L),
    MakeIsotopeMassRow(10U, 21U, 0U, 20.993846685L, 0.0L),
    MakeIsotopeMassRow(10U, 22U, 0U, 21.991385113L, 0.0L),
    MakeIsotopeMassRow(11U, 23U, 0U, 22.98976928195L, 0.0L),
    MakeIsotopeMassRow(12U, 24U, 0U, 23.985041689L, 0.0L),
    MakeIsotopeMassRow(12U, 25U, 0U, 24.985836966L, 0.0L),
    MakeIsotopeMassRow(12U, 26U, 0U, 25.982592972L, 0.0L),
    MakeIsotopeMassRow(13U, 27U, 0U, 26.981538408L, 0.0L),
    MakeIsotopeMassRow(14U, 28U, 0U, 27.97692653442L, 0.0L),
    MakeIsotopeMassRow(14U, 29U, 0U, 28.97649466434L, 0.0L),
    MakeIsotopeMassRow(14U, 30U, 0U, 29.973770137L, 0.0L),
    MakeIsotopeMassRow(15U, 31U, 0U, 30.97376199768L, 0.0L),
    MakeIsotopeMassRow(16U, 32U, 0U, 31.97207117354L, 0.0L),
    MakeIsotopeMassRow(16U, 33U, 0U, 32.97145890862L, 0.0L),
    MakeIsotopeMassRow(16U, 34U, 0U, 33.967867011L, 0.0L),
    MakeIsotopeMassRow(16U, 36U, 0U, 35.967080692L, 0.0L),
    MakeIsotopeMassRow(17U, 35U, 0U, 34.968852694L, 0.0L),
    MakeIsotopeMassRow(17U, 37U, 0U, 36.965902573L, 0.0L),
    MakeIsotopeMassRow(18U, 36U, 0U, 35.967545106L, 0.0L),
    MakeIsotopeMassRow(18U, 38U, 0U, 37.962732102L, 0.0L),
    MakeIsotopeMassRow(18U, 40U, 0U, 39.96238312204L, 0.0L),
    MakeIsotopeMassRow(19U, 39U, 0U, 38.96370648482L, 0.0L),
    MakeIsotopeMassRow(19U, 40U, 0U, 39.963998165L, 0.0L),
    MakeIsotopeMassRow(19U, 41U, 0U, 40.96182525611L, 0.0L),
    MakeIsotopeMassRow(20U, 40U, 0U, 39.96259085L, 0.0L),
    MakeIsotopeMassRow(20U, 42U, 0U, 41.95861778L, 0.0L),
    MakeIsotopeMassRow(20U, 43U, 0U, 42.958766381L, 0.0L),
    MakeIsotopeMassRow(20U, 44U, 0U, 43.955481489L, 0.0L),
    MakeIsotopeMassRow(20U, 46U, 0U, 45.953687726L, 0.0L),
    MakeIsotopeMassRow(20U, 48U, 0U, 47.952522654L, 0.0L),
    MakeIsotopeMassRow(21U, 45U, 0U, 44.955907051L, 0.0L),
    MakeIsotopeMassRow(22U, 46U, 0U, 45.952626356L, 0.0L),
    MakeIsotopeMassRow(22U, 47U, 0U, 46.951757491L, 0.0L),
    MakeIsotopeMassRow(22U, 48U, 0U, 47.947940677L, 0.0L),
    MakeIsotopeMassRow(22U, 49U, 0U, 48.947864391L, 0.0L),
    MakeIsotopeMassRow(22U, 50U, 0U, 49.944785622L, 0.0L),
    MakeIsotopeMassRow(23U, 50U, 0U, 49.947156681L, 0.0L),
    MakeIsotopeMassRow(23U, 51U, 0U, 50.943957664L, 0.0L),
    MakeIsotopeMassRow(24U, 50U, 0U, 49.946042209L, 0.0L),
    MakeIsotopeMassRow(24U, 52U, 0U, 51.940504714L, 0.0L),
    MakeIsotopeMassRow(24U, 53U, 0U, 52.940646304L, 0.0L),
    MakeIsotopeMassRow(24U, 54U, 0U, 53.938877359L, 0.0L),
    MakeIsotopeMassRow(25U, 55U, 0U, 54.93804304L, 0.0L),
    MakeIsotopeMassRow(26U, 54U, 0U, 53.939608189L, 0.0L),
    MakeIsotopeMassRow(26U, 56U, 0U, 55.934935537L, 0.0L),
    MakeIsotopeMassRow(26U, 57U, 0U, 56.93539195L, 0.0L),
    MakeIsotopeMassRow(26U, 58U, 0U, 57.933273575L, 0.0L),
    MakeIsotopeMassRow(27U, 59U, 0U, 58.933193524L, 0.0L),
    MakeIsotopeMassRow(28U, 58U, 0U, 57.93534165L, 0.0L),
    MakeIsotopeMassRow(28U, 60U, 0U, 59.930785129L, 0.0L),
    MakeIsotopeMassRow(28U, 61U, 0U, 60.931054819L, 0.0L),
    MakeIsotopeMassRow(28U, 62U, 0U, 61.928344753L, 0.0L),
    MakeIsotopeMassRow(28U, 64U, 0U, 63.927966228L, 0.0L),
    MakeIsotopeMassRow(29U, 63U, 0U, 62.929597119L, 0.0L),
    MakeIsotopeMassRow(29U, 65U, 0U, 64.927789476L, 0.0L),
    MakeIsotopeMassRow(30U, 64U, 0U, 63.929141776L, 0.0L),
    MakeIsotopeMassRow(30U, 66U, 0U, 65.926033639L, 0.0L),
    MakeIsotopeMassRow(30U, 67U, 0U, 66.927127422L, 0.0L),
    MakeIsotopeMassRow(30U, 68U, 0U, 67.924844232L, 0.0L),
    MakeIsotopeMassRow(30U, 70U, 0U, 69.925319175L, 0.0L),
    MakeIsotopeMassRow(31U, 69U, 0U, 68.925573528L, 0.0L),
    MakeIsotopeMassRow(31U, 71U, 0U, 70.924702554L, 0.0L),
    MakeIsotopeMassRow(32U, 70U, 0U, 69.924248542L, 0.0L),
    MakeIsotopeMassRow(32U, 72U, 0U, 71.922075824L, 0.0L),
    MakeIsotopeMassRow(32U, 73U, 0U, 72.923458954L, 0.0L),
    MakeIsotopeMassRow(32U, 74U, 0U, 73.92117776L, 0.0L),
    MakeIsotopeMassRow(32U, 76U, 0U, 75.921402725L, 0.0L),
    MakeIsotopeMassRow(33U, 75U, 0U, 74.921594562L, 0.0L),
    MakeIsotopeMassRow(34U, 74U, 0U, 73.922475933L, 0.0L),
    MakeIsotopeMassRow(34U, 76U, 0U, 75.919213702L, 0.0L),
    MakeIsotopeMassRow(34U, 77U, 0U, 76.91991415L, 0.0L),
    MakeIsotopeMassRow(34U, 78U, 0U, 77.917309244L, 0.0L),
    MakeIsotopeMassRow(34U, 80U, 0U, 79.916521761L, 0.0L),
    MakeIsotopeMassRow(34U, 82U, 0U, 81.916699531L, 0.0L),
    MakeIsotopeMassRow(35U, 79U, 0U, 78.918337574L, 0.0L),
    MakeIsotopeMassRow(35U, 81U, 0U, 80.916288197L, 0.0L),
    MakeIsotopeMassRow(36U, 78U, 0U, 77.920366341L, 0.0L),
    MakeIsotopeMassRow(36U, 80U, 0U, 79.91637794L, 0.0L),
    MakeIsotopeMassRow(36U, 82U, 0U, 81.91348115368L, 0.0L),
    MakeIsotopeMassRow(36U, 83U, 0U, 82.914126516L, 0.0L),
    MakeIsotopeMassRow(36U, 84U, 0U, 83.91149772708L, 0.0L),
    MakeIsotopeMassRow(36U, 86U, 0U, 85.91061062468L, 0.0L),
    MakeIsotopeMassRow(37U, 85U, 0U, 84.91178973604L, 0.0L),
    MakeIsotopeMassRow(37U, 87U, 0U, 86.909180529L, 0.0L),
    MakeIsotopeMassRow(38U, 84U, 0U, 83.913419118L, 0.0L),
    MakeIsotopeMassRow(38U, 86U, 0U, 85.90926072473L, 0.0L),
    MakeIsotopeMassRow(38U, 87U, 0U, 86.90887749454L, 0.0L),
    MakeIsotopeMassRow(38U, 88U, 0U, 87.905612253L, 0.0L),
    MakeIsotopeMassRow(39U, 89U, 0U, 88.905838156L, 0.0L),
    MakeIsotopeMassRow(40U, 90U, 0U, 89.904698755L, 0.0L),
    MakeIsotopeMassRow(40U, 91U, 0U, 90.905640205L, 0.0L),
    MakeIsotopeMassRow(40U, 92U, 0U, 91.905035336L, 0.0L),
    MakeIsotopeMassRow(40U, 94U, 0U, 93.906312523L, 0.0L),
    MakeIsotopeMassRow(40U, 96U, 0U, 95.908277615L, 0.0L),
    MakeIsotopeMassRow(41U, 93U, 0U, 92.90637317L, 0.0L),
    MakeIsotopeMassRow(42U, 92U, 0U, 91.906807153L, 0.0L),
    MakeIsotopeMassRow(42U, 94U, 0U, 93.905083586L, 0.0L),
    MakeIsotopeMassRow(42U, 95U, 0U, 94.905837436L, 0.0L),
    MakeIsotopeMassRow(42U, 96U, 0U, 95.90467477L, 0.0L),
    MakeIsotopeMassRow(42U, 97U, 0U, 96.906016903L, 0.0L),
    MakeIsotopeMassRow(42U, 98U, 0U, 97.905403609L, 0.0L),
    MakeIsotopeMassRow(42U, 100U, 0U, 99.907467982L, 0.0L),
    MakeIsotopeMassRow(43U, 97U, 0U, 96.90636072L, 0.0L),
    MakeIsotopeMassRow(44U, 96U, 0U, 95.90758891L, 0.0L),
    MakeIsotopeMassRow(44U, 98U, 0U, 97.905286709L, 0.0L),
    MakeIsotopeMassRow(44U, 99U, 0U, 98.905930284L, 0.0L),
    MakeIsotopeMassRow(44U, 100U, 0U, 99.90421046L, 0.0L),
    MakeIsotopeMassRow(44U, 101U, 0U, 100.905573086L, 0.0L),
    MakeIsotopeMassRow(44U, 102U, 0U, 101.904340312L, 0.0L),
    MakeIsotopeMassRow(44U, 104U, 0U, 103.905425312L, 0.0L),
    MakeIsotopeMassRow(45U, 103U, 0U, 102.905494081L, 0.0L),
    MakeIsotopeMassRow(46U, 102U, 0U, 101.905632292L, 0.0L),
    MakeIsotopeMassRow(46U, 104U, 0U, 103.904030393L, 0.0L),
    MakeIsotopeMassRow(46U, 105U, 0U, 104.905079479L, 0.0L),
    MakeIsotopeMassRow(46U, 106U, 0U, 105.903480287L, 0.0L),
    MakeIsotopeMassRow(46U, 108U, 0U, 107.903891806L, 0.0L),
    MakeIsotopeMassRow(46U, 110U, 0U, 109.905172878L, 0.0L),
    MakeIsotopeMassRow(47U, 107U, 0U, 106.905091509L, 0.0L),
    MakeIsotopeMassRow(47U, 109U, 0U, 108.904755778L, 0.0L),
    MakeIsotopeMassRow(48U, 106U, 0U, 105.906459791L, 0.0L),
    MakeIsotopeMassRow(48U, 108U, 0U, 107.904183588L, 0.0L),
    MakeIsotopeMassRow(48U, 110U, 0U, 109.90300747L, 0.0L),
    MakeIsotopeMassRow(48U, 111U, 0U, 110.904183776L, 0.0L),
    MakeIsotopeMassRow(48U, 112U, 0U, 111.902763896L, 0.0L),
    MakeIsotopeMassRow(48U, 113U, 0U, 112.904408105L, 0.0L),
    MakeIsotopeMassRow(48U, 114U, 0U, 113.903364998L, 0.0L),
    MakeIsotopeMassRow(48U, 116U, 0U, 115.90476323L, 0.0L),
    MakeIsotopeMassRow(49U, 113U, 0U, 112.904060451L, 0.0L),
    MakeIsotopeMassRow(49U, 115U, 0U, 114.903878772L, 0.0L),
    MakeIsotopeMassRow(50U, 112U, 0U, 111.904824894L, 0.0L),
    MakeIsotopeMassRow(50U, 114U, 0U, 113.90278013L, 0.0L),
    MakeIsotopeMassRow(50U, 115U, 0U, 114.903344695L, 0.0L),
    MakeIsotopeMassRow(50U, 116U, 0U, 115.901742825L, 0.0L),
    MakeIsotopeMassRow(50U, 117U, 0U, 116.902954036L, 0.0L),
    MakeIsotopeMassRow(50U, 118U, 0U, 117.90160663L, 0.0L),
    MakeIsotopeMassRow(50U, 119U, 0U, 118.903311266L, 0.0L),
    MakeIsotopeMassRow(50U, 120U, 0U, 119.902202557L, 0.0L),
    MakeIsotopeMassRow(50U, 122U, 0U, 121.903445494L, 0.0L),
    MakeIsotopeMassRow(50U, 124U, 0U, 123.905279619L, 0.0L),
    MakeIsotopeMassRow(51U, 121U, 0U, 120.903811353L, 0.0L),
    MakeIsotopeMassRow(51U, 123U, 0U, 122.904215292L, 0.0L),
    MakeIsotopeMassRow(52U, 120U, 0U, 119.904065779L, 0.0L),
    MakeIsotopeMassRow(52U, 122U, 0U, 121.903044708L, 0.0L),
    MakeIsotopeMassRow(52U, 123U, 0U, 122.904271022L, 0.0L),
    MakeIsotopeMassRow(52U, 124U, 0U, 123.902818341L, 0.0L),
    MakeIsotopeMassRow(52U, 125U, 0U, 124.904431178L, 0.0L),
    MakeIsotopeMassRow(52U, 126U, 0U, 125.903312144L, 0.0L),
    MakeIsotopeMassRow(52U, 128U, 0U, 127.904461237L, 0.0L),
    MakeIsotopeMassRow(52U, 130U, 0U, 129.906222745L, 0.0L),
    MakeIsotopeMassRow(53U, 127U, 0U, 126.904472592L, 0.0L),
    MakeIsotopeMassRow(54U, 124U, 0U, 123.905885174L, 0.0L),
    MakeIsotopeMassRow(54U, 126U, 0U, 125.904297422L, 0.0L),
    MakeIsotopeMassRow(54U, 128U, 0U, 127.90353075341L, 0.0L),
    MakeIsotopeMassRow(54U, 129U, 0U, 128.90478085742L, 0.0L),
    MakeIsotopeMassRow(54U, 130U, 0U, 129.903509346L, 0.0L),
    MakeIsotopeMassRow(54U, 131U, 0U, 130.90508412808L, 0.0L),
    MakeIsotopeMassRow(54U, 132U, 0U, 131.90415508346L, 0.0L),
    MakeIsotopeMassRow(54U, 134U, 0U, 133.90539303L, 0.0L),
    MakeIsotopeMassRow(54U, 136U, 0U, 135.907214474L, 0.0L),
    MakeIsotopeMassRow(55U, 133U, 0U, 132.905451958L, 0.0L),
    MakeIsotopeMassRow(56U, 130U, 0U, 129.906326002L, 0.0L),
    MakeIsotopeMassRow(56U, 132U, 0U, 131.905061231L, 0.0L),
    MakeIsotopeMassRow(56U, 134U, 0U, 133.904508249L, 0.0L),
    MakeIsotopeMassRow(56U, 135U, 0U, 134.905688447L, 0.0L),
    MakeIsotopeMassRow(56U, 136U, 0U, 135.9045758L, 0.0L),
    MakeIsotopeMassRow(56U, 137U, 0U, 136.905827207L, 0.0L),
    MakeIsotopeMassRow(56U, 138U, 0U, 137.905247059L, 0.0L),
    MakeIsotopeMassRow(57U, 138U, 0U, 137.907124041L, 0.0L),
    MakeIsotopeMassRow(57U, 139U, 0U, 138.906362927L, 0.0L),
    MakeIsotopeMassRow(58U, 136U, 0U, 135.907129256L, 0.0L),
    MakeIsotopeMassRow(58U, 138U, 0U, 137.90599418L, 0.0L),
    MakeIsotopeMassRow(58U, 140U, 0U, 139.905448433L, 0.0L),
    MakeIsotopeMassRow(58U, 142U, 0U, 141.909250208L, 0.0L),
    MakeIsotopeMassRow(59U, 141U, 0U, 140.907659604L, 0.0L),
    MakeIsotopeMassRow(60U, 142U, 0U, 141.907728824L, 0.0L),
    MakeIsotopeMassRow(60U, 143U, 0U, 142.909819815L, 0.0L),
    MakeIsotopeMassRow(60U, 144U, 0U, 143.910092798L, 0.0L),
    MakeIsotopeMassRow(60U, 145U, 0U, 144.912579151L, 0.0L),
    MakeIsotopeMassRow(60U, 146U, 0U, 145.913122459L, 0.0L),
    MakeIsotopeMassRow(60U, 148U, 0U, 147.916899027L, 0.0L),
    MakeIsotopeMassRow(60U, 150U, 0U, 149.920901322L, 0.0L),
    MakeIsotopeMassRow(61U, 145U, 0U, 144.912755748L, 0.0L),
    MakeIsotopeMassRow(62U, 144U, 0U, 143.912006285L, 0.0L),
    MakeIsotopeMassRow(62U, 147U, 0U, 146.914904401L, 0.0L),
    MakeIsotopeMassRow(62U, 148U, 0U, 147.914829233L, 0.0L),
    MakeIsotopeMassRow(62U, 149U, 0U, 148.917191211L, 0.0L),
    MakeIsotopeMassRow(62U, 150U, 0U, 149.917281993L, 0.0L),
    MakeIsotopeMassRow(62U, 152U, 0U, 151.919738646L, 0.0L),
    MakeIsotopeMassRow(62U, 154U, 0U, 153.922215756L, 0.0L),
    MakeIsotopeMassRow(63U, 151U, 0U, 150.919856606L, 0.0L),
    MakeIsotopeMassRow(63U, 153U, 0U, 152.921236789L, 0.0L),
    MakeIsotopeMassRow(64U, 152U, 0U, 151.919798414L, 0.0L),
    MakeIsotopeMassRow(64U, 154U, 0U, 153.920872974L, 0.0L),
    MakeIsotopeMassRow(64U, 155U, 0U, 154.922629356L, 0.0L),
    MakeIsotopeMassRow(64U, 156U, 0U, 155.92213012L, 0.0L),
    MakeIsotopeMassRow(64U, 157U, 0U, 156.923967424L, 0.0L),
    MakeIsotopeMassRow(64U, 158U, 0U, 157.9241112L, 0.0L),
    MakeIsotopeMassRow(64U, 160U, 0U, 159.927061202L, 0.0L),
    MakeIsotopeMassRow(65U, 159U, 0U, 158.925353707L, 0.0L),
    MakeIsotopeMassRow(66U, 156U, 0U, 155.924283593L, 0.0L),
    MakeIsotopeMassRow(66U, 158U, 0U, 157.924414817L, 0.0L),
    MakeIsotopeMassRow(66U, 160U, 0U, 159.925203578L, 0.0L),
    MakeIsotopeMassRow(66U, 161U, 0U, 160.926939425L, 0.0L),
    MakeIsotopeMassRow(66U, 162U, 0U, 161.926804507L, 0.0L),
    MakeIsotopeMassRow(66U, 163U, 0U, 162.928737221L, 0.0L),
    MakeIsotopeMassRow(66U, 164U, 0U, 163.929180819L, 0.0L),
    MakeIsotopeMassRow(67U, 165U, 0U, 164.930329116L, 0.0L),
    MakeIsotopeMassRow(68U, 162U, 0U, 161.928787299L, 0.0L),
    MakeIsotopeMassRow(68U, 164U, 0U, 163.929207739L, 0.0L),
    MakeIsotopeMassRow(68U, 166U, 0U, 165.930301067L, 0.0L),
    MakeIsotopeMassRow(68U, 167U, 0U, 166.932056192L, 0.0L),
    MakeIsotopeMassRow(68U, 168U, 0U, 167.932378282L, 0.0L),
    MakeIsotopeMassRow(68U, 170U, 0U, 169.935471933L, 0.0L),
    MakeIsotopeMassRow(69U, 169U, 0U, 168.934218956L, 0.0L),
    MakeIsotopeMassRow(70U, 168U, 0U, 167.933891297L, 0.0L),
    MakeIsotopeMassRow(70U, 170U, 0U, 169.934767242L, 0.0L),
    MakeIsotopeMassRow(70U, 171U, 0U, 170.936331515L, 0.0L),
    MakeIsotopeMassRow(70U, 172U, 0U, 171.936386654L, 0.0L),
    MakeIsotopeMassRow(70U, 173U, 0U, 172.938216211L, 0.0L),
    MakeIsotopeMassRow(70U, 174U, 0U, 173.938867545L, 0.0L),
    MakeIsotopeMassRow(70U, 176U, 0U, 175.942574706L, 0.0L),
    MakeIsotopeMassRow(71U, 175U, 0U, 174.940777211L, 0.0L),
    MakeIsotopeMassRow(71U, 176U, 0U, 175.942691711L, 0.0L),
    MakeIsotopeMassRow(72U, 174U, 0U, 173.940048377L, 0.0L),
    MakeIsotopeMassRow(72U, 176U, 0U, 175.941409797L, 0.0L),
    MakeIsotopeMassRow(72U, 177U, 0U, 176.943230187L, 0.0L),
    MakeIsotopeMassRow(72U, 178U, 0U, 177.943708322L, 0.0L),
    MakeIsotopeMassRow(72U, 179U, 0U, 178.945825705L, 0.0L),
    MakeIsotopeMassRow(72U, 180U, 0U, 179.946559537L, 0.0L),
    MakeIsotopeMassRow(73U, 180U, 1U, 179.947467589L, 75.3L),
    MakeIsotopeMassRow(73U, 181U, 0U, 180.947998528L, 0.0L),
    MakeIsotopeMassRow(74U, 180U, 0U, 179.946713304L, 0.0L),
    MakeIsotopeMassRow(74U, 182U, 0U, 181.948205636L, 0.0L),
    MakeIsotopeMassRow(74U, 183U, 0U, 182.950224416L, 0.0L),
    MakeIsotopeMassRow(74U, 184U, 0U, 183.95093318L, 0.0L),
    MakeIsotopeMassRow(74U, 186U, 0U, 185.95436514L, 0.0L),
    MakeIsotopeMassRow(75U, 185U, 0U, 184.95295832L, 0.0L),
    MakeIsotopeMassRow(75U, 187U, 0U, 186.955752217L, 0.0L),
    MakeIsotopeMassRow(76U, 184U, 0U, 183.952492919L, 0.0L),
    MakeIsotopeMassRow(76U, 186U, 0U, 185.953837569L, 0.0L),
    MakeIsotopeMassRow(76U, 187U, 0U, 186.955749569L, 0.0L),
    MakeIsotopeMassRow(76U, 188U, 0U, 187.955837292L, 0.0L),
    MakeIsotopeMassRow(76U, 189U, 0U, 188.958145949L, 0.0L),
    MakeIsotopeMassRow(76U, 190U, 0U, 189.958445442L, 0.0L),
    MakeIsotopeMassRow(76U, 192U, 0U, 191.961478765L, 0.0L),
    MakeIsotopeMassRow(77U, 191U, 0U, 190.960591455L, 0.0L),
    MakeIsotopeMassRow(77U, 193U, 0U, 192.962923753L, 0.0L),
    MakeIsotopeMassRow(78U, 190U, 0U, 189.959949823L, 0.0L),
    MakeIsotopeMassRow(78U, 192U, 0U, 191.961042667L, 0.0L),
    MakeIsotopeMassRow(78U, 194U, 0U, 193.962683498L, 0.0L),
    MakeIsotopeMassRow(78U, 195U, 0U, 194.964794325L, 0.0L),
    MakeIsotopeMassRow(78U, 196U, 0U, 195.964954648L, 0.0L),
    MakeIsotopeMassRow(78U, 198U, 0U, 197.967896718L, 0.0L),
    MakeIsotopeMassRow(79U, 197U, 0U, 196.966570103L, 0.0L),
    MakeIsotopeMassRow(80U, 196U, 0U, 195.965833445L, 0.0L),
    MakeIsotopeMassRow(80U, 198U, 0U, 197.966769177L, 0.0L),
    MakeIsotopeMassRow(80U, 199U, 0U, 198.968280994L, 0.0L),
    MakeIsotopeMassRow(80U, 200U, 0U, 199.968326941L, 0.0L),
    MakeIsotopeMassRow(80U, 201U, 0U, 200.970303054L, 0.0L),
    MakeIsotopeMassRow(80U, 202U, 0U, 201.970643604L, 0.0L),
    MakeIsotopeMassRow(80U, 204U, 0U, 203.973494037L, 0.0L),
    MakeIsotopeMassRow(81U, 203U, 0U, 202.972344098L, 0.0L),
    MakeIsotopeMassRow(81U, 205U, 0U, 204.974427318L, 0.0L),
    MakeIsotopeMassRow(82U, 204U, 0U, 203.973043506L, 0.0L),
    MakeIsotopeMassRow(82U, 206U, 0U, 205.97446521L, 0.0L),
    MakeIsotopeMassRow(82U, 207U, 0U, 206.975896821L, 0.0L),
    MakeIsotopeMassRow(82U, 208U, 0U, 207.976652005L, 0.0L),
    MakeIsotopeMassRow(83U, 209U, 0U, 208.980398599L, 0.0L),
    MakeIsotopeMassRow(84U, 209U, 0U, 208.982430361L, 0.0L),
    MakeIsotopeMassRow(85U, 210U, 0U, 209.987147423L, 0.0L),
    MakeIsotopeMassRow(86U, 222U, 0U, 222.017576017L, 0.0L),
    MakeIsotopeMassRow(87U, 223U, 0U, 223.019734241L, 0.0L),
    MakeIsotopeMassRow(88U, 226U, 0U, 226.025408186L, 0.0L),
    MakeIsotopeMassRow(89U, 227U, 0U, 227.027750594L, 0.0L),
    MakeIsotopeMassRow(90U, 232U, 0U, 232.038053606L, 0.0L),
    MakeIsotopeMassRow(91U, 231U, 0U, 231.0358825L, 0.0L),
    MakeIsotopeMassRow(92U, 234U, 0U, 234.040950296L, 0.0L),
    MakeIsotopeMassRow(92U, 235U, 0U, 235.043928117L, 0.0L),
    MakeIsotopeMassRow(92U, 238U, 0U, 238.050786936L, 0.0L),
};

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildIsotopeMassAuthority() -> GGEMSResolvedIsotopeTable {
  std::vector<GGEMSResolvedIsotope> resolved_isotopes;
  resolved_isotopes.reserve(k_isotope_masses.size());

  for (auto const &row : k_isotope_masses) {
    long double const relative_atomic_mass =
        row.ground_state_relative_atomic_mass +
        (row.excitation_energy_kilo_electron_volts /
         k_atomic_mass_unit_kilo_electron_volts);

    resolved_isotopes.push_back({
        .isotope = row.isotope,
        .molar_mass_grams_per_mole =
            relative_atomic_mass * k_molar_mass_constant_grams_per_mole,
    });
  }

  return GGEMSResolvedIsotopeTable{std::move(resolved_isotopes)};
}

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto GetIsotopeMassAuthority()
    -> GGEMSResolvedIsotopeTable const & {
  static GGEMSResolvedIsotopeTable const authority =
      BuildIsotopeMassAuthority();
  return authority;
}

} // namespace ggems::core::materials
