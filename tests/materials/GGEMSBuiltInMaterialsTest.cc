#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSIsotopeProfile.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace {

namespace materials = ggems::core::materials;
namespace builtins = ggems::core::materials::builtins;

using namespace ggems::units;

constexpr std::array<std::string_view, 24U> k_compound_names{{
    "Air",           "Water",         "Adipose",       "Blood", "BloodIodine5",
    "BloodIodine10", "BloodIodine15", "BloodIodine20", "Brain", "Breast",
    "Heart",         "Intestine",     "Kidney",        "Liver", "Lung",
    "RibBone",       "SpineBone",     "Spleen",        "CdTe",  "CsI",
    "GaAs",          "GOS",           "LSO",           "NaI",
}};

constexpr std::array<std::uint32_t, 8U> k_reference_elements{
    43U, 61U, 84U, 85U, 86U, 87U, 88U, 89U,
};

struct ExpectedTotals {
  std::string_view name;
  long double total_atom_density;
  long double electron_density;
};

constexpr std::array<ExpectedTotals, 116U> k_expected_totals{
    {
        {
            .name = "Hydrogen",
            .total_atom_density = 5.00380886730810082477103861468017303e+19L,
            .electron_density = 5.00380886730810082477103861468017303e+19L,
        },
        {
            .name = "Helium",
            .total_atom_density = 2.50207745941343347903004144541541994e+19L,
            .electron_density = 5.00415491882686695806008289083083988e+19L,
        },
        {
            .name = "Lithium",
            .total_atom_density = 4.63372651682213262899368059297536288e+22L,
            .electron_density = 1.39011795504663978869810417789260886e+23L,
        },
        {
            .name = "Beryllium",
            .total_atom_density = 1.23487461764063288415763319459700612e+23L,
            .electron_density = 4.93949847056253153663053277838802449e+23L,
        },
        {
            .name = "Boron",
            .total_atom_density = 1.32017729981006898245475713626842762e+23L,
            .electron_density = 6.60088649905034491227378568134213808e+23L,
        },
        {
            .name = "Carbon",
            .total_atom_density = 8.52374023479344441472681014169133056e+22L,
            .electron_density = 5.11424414087606664883608608501479833e+23L,
        },
        {
            .name = "Nitrogen",
            .total_atom_density = 5.00888315559602471608143188550773764e+19L,
            .electron_density = 3.50621820891721730125700231985541635e+20L,
        },
        {
            .name = "Oxygen",
            .total_atom_density = 5.01361864519165090253988228294364848e+19L,
            .electron_density = 4.01089491615332072203190582635491879e+20L,
        },
        {
            .name = "Fluorine",
            .total_atom_density = 5.00830638745774940563294897406588043e+19L,
            .electron_density = 4.50747574871197446506965407665929238e+20L,
        },
        {
            .name = "Neon",
            .total_atom_density = 2.50225640069416141665285871690831948e+19L,
            .electron_density = 2.50225640069416141665285871690831948e+20L,
        },
        {
            .name = "Sodium",
            .total_atom_density = 2.54352212069008618648229447534396566e+22L,
            .electron_density = 2.79787433275909480513052392287836223e+23L,
        },
        {
            .name = "Magnesium",
            .total_atom_density = 4.31125392295439383969486472743139580e+22L,
            .electron_density = 5.17350470754527260763383767291767496e+23L,
        },
        {
            .name = "Aluminum",
            .total_atom_density = 6.02402933753930455688409688041326288e+22L,
            .electron_density = 7.83123813880109592394932594453724174e+23L,
        },
        {
            .name = "Silicon",
            .total_atom_density = 4.99602592184690493075558067883306064e+22L,
            .electron_density = 6.99443629058566690305781295036628490e+23L,
        },
        {
            .name = "Phosphorus",
            .total_atom_density = 4.27739764355431255406149983027093397e+22L,
            .electron_density = 6.41609646533146883109224974540640096e+23L,
        },
        {
            .name = "Sulfur",
            .total_atom_density = 3.75623307742596860827361565546944473e+22L,
            .electron_density = 6.00997292388154977323778504875111157e+23L,
        },
        {
            .name = "Chlorine",
            .total_atom_density = 5.08739551248386640923467511247765860e+19L,
            .electron_density = 8.64857237122257289569894769121201962e+20L,
        },
        {
            .name = "Argon",
            .total_atom_density = 2.50546921054865431527376934928865902e+19L,
            .electron_density = 4.50984457898757776749278482871958623e+20L,
        },
        {
            .name = "Potassium",
            .total_atom_density = 1.32770100210208484893399283246638127e+22L,
            .electron_density = 2.52263190399396121297458638168612441e+23L,
        },
        {
            .name = "Calcium",
            .total_atom_density = 2.32903661079208671192471455592557165e+22L,
            .electron_density = 4.65807322158417342384942911185114329e+23L,
        },
        {
            .name = "Scandium",
            .total_atom_density = 4.00396297027654256941425515600940758e+22L,
            .electron_density = 8.40832223758073939576993582761975591e+23L,
        },
        {
            .name = "Titanium",
            .total_atom_density = 5.71179840794868095866715733584338518e+22L,
            .electron_density = 1.25659564974870981090677461388554474e+24L,
        },
        {
            .name = "Vanadium",
            .total_atom_density = 7.22305091287214455687812024126504372e+22L,
            .electron_density = 1.66130170996059324808196765549096005e+24L,
        },
        {
            .name = "Chromium",
            .total_atom_density = 8.31580549850499581326455063434519065e+22L,
            .electron_density = 1.99579331964119899518349215224284576e+24L,
        },
        {
            .name = "Manganese",
            .total_atom_density = 8.15550112965127496690628602815061752e+22L,
            .electron_density = 2.03887528241281874172657150703765438e+24L,
        },
        {
            .name = "Iron",
            .total_atom_density = 8.49104025977329906162211186190512133e+22L,
            .electron_density = 2.20767046754105775602174908409533155e+24L,
        },
        {
            .name = "Cobalt",
            .total_atom_density = 9.09454409354147571052486896115769229e+22L,
            .electron_density = 2.45552690525619844184171461951257692e+24L,
        },
        {
            .name = "Nickel",
            .total_atom_density = 9.13376052221999815440974928901330984e+22L,
            .electron_density = 2.55745294622159948323472980092372676e+24L,
        },
        {
            .name = "Copper",
            .total_atom_density = 8.49122645730663922447192541385553867e+22L,
            .electron_density = 2.46245567261892537509685837001810621e+24L,
        },
        {
            .name = "Zinc",
            .total_atom_density = 6.57041711848926294026881462283619590e+22L,
            .electron_density = 1.97112513554677888208064438685085877e+24L,
        },
        {
            .name = "Gallium",
            .total_atom_density = 5.09941989203338414645504912545287175e+22L,
            .electron_density = 1.58082016653034908540106522889039024e+24L,
        },
        {
            .name = "Germanium",
            .total_atom_density = 4.41373214138758276584894280199480642e+22L,
            .electron_density = 1.41239428524402648507166169663833805e+24L,
        },
        {
            .name = "Arsenic",
            .total_atom_density = 4.60573039325961778847274667228150881e+22L,
            .electron_density = 1.51989102977567387019600640185289791e+24L,
        },
        {
            .name = "Selenium",
            .total_atom_density = 3.43209767649078925193948819726524469e+22L,
            .electron_density = 1.16691321000686834565942598707018320e+24L,
        },
        {
            .name = "Bromine",
            .total_atom_density = 5.32999993805488697411344029807887547e+19L,
            .electron_density = 1.86549997831921044093970410432760641e+21L,
        },
        {
            .name = "Krypton",
            .total_atom_density = 2.49946366338756393044703863718090583e+19L,
            .electron_density = 8.99806918819523014960933909385126100e+20L,
        },
        {
            .name = "Rubidium",
            .total_atom_density = 1.07946318487061162169896059176198723e+22L,
            .electron_density = 3.99401378402126300028615418951935274e+23L,
        },
        {
            .name = "Strontium",
            .total_atom_density = 1.74581412475281652018463313626114606e+22L,
            .electron_density = 6.63409367406070277670160591779235504e+23L,
        },
        {
            .name = "Yttrium",
            .total_atom_density = 3.02712932990499319897211139212309792e+22L,
            .electron_density = 1.18058043866294734759912344292800819e+24L,
        },
        {
            .name = "Zirconium",
            .total_atom_density = 4.29494444038923907898034057407783081e+22L,
            .electron_density = 1.71797777615569563159213622963113232e+24L,
        },
        {
            .name = "Niobium",
            .total_atom_density = 5.55502755064760714176522894081220623e+22L,
            .electron_density = 2.27756129576551892812374386573300455e+24L,
        },
        {
            .name = "Molybdenum",
            .total_atom_density = 6.41375730139135106099895862826741830e+22L,
            .electron_density = 2.69377806658436744561956262387231569e+24L,
        },
        {
            .name = "Technetium",
            .total_atom_density = 7.14655035569708993188504263135629618e+22L,
            .electron_density = 3.07301665294974867071056833148320736e+24L,
        },
        {
            .name = "Ruthenium",
            .total_atom_density = 7.39472749802257863263769912719374589e+22L,
            .electron_density = 3.25368009912993459836058761596524819e+24L,
        },
        {
            .name = "Rhodium",
            .total_atom_density = 7.26246615115637257276549904186103255e+22L,
            .electron_density = 3.26810976802036765774447456883746465e+24L,
        },
        {
            .name = "Palladium",
            .total_atom_density = 6.80222795929287929547812255362118043e+22L,
            .electron_density = 3.12902486127472447591993637466574300e+24L,
        },
        {
            .name = "Silver",
            .total_atom_density = 5.86201562004475962117860307411839892e+22L,
            .electron_density = 2.75514734142103702195394344483564749e+24L,
        },
        {
            .name = "Cadmium",
            .total_atom_density = 4.63400012621664878862940729447266372e+22L,
            .electron_density = 2.22432006058399141854211550134687858e+24L,
        },
        {
            .name = "Indium",
            .total_atom_density = 3.83405177929534296943078522531413037e+22L,
            .electron_density = 1.87868537185471805502108476040392388e+24L,
        },
        {
            .name = "Tin",
            .total_atom_density = 3.70834865986683766232150419054061356e+22L,
            .electron_density = 1.85417432993341883116075209527030678e+24L,
        },
        {
            .name = "Antimony",
            .total_atom_density = 3.30931465163915550004202276543988021e+22L,
            .electron_density = 1.68775047233596930502143161037433891e+24L,
        },
        {
            .name = "Tellurium",
            .total_atom_density = 2.94492456537397104645103850853948957e+22L,
            .electron_density = 1.53136077399446494415454002444053458e+24L,
        },
        {
            .name = "Iodine",
            .total_atom_density = 2.33948838123913207875256540445841981e+22L,
            .electron_density = 1.23992884205674000173885966436296250e+24L,
        },
        {
            .name = "Xenon",
            .total_atom_density = 2.51586162957431524861517794324704754e+19L,
            .electron_density = 1.35856527997013023425219608935340567e+21L,
        },
        {
            .name = "Cesium",
            .total_atom_density = 8.48683742123763937366649473929725002e+21L,
            .electron_density = 4.66776058168070165551657210661348751e+23L,
        },
        {
            .name = "Barium",
            .total_atom_density = 1.53484087330538317226799505017902132e+22L,
            .electron_density = 8.59510889051014576470077228100251941e+23L,
        },
        {
            .name = "Lanthanum",
            .total_atom_density = 2.66801967769376446770022284786211950e+22L,
            .electron_density = 1.52077121628544574658912702328140812e+24L,
        },
        {
            .name = "Cerium",
            .total_atom_density = 2.86116265748962329850139568724717001e+22L,
            .electron_density = 1.65947434134398151313080949860335861e+24L,
        },
        {
            .name = "Praseodymium",
            .total_atom_density = 2.86773370381235924228248970342830389e+22L,
            .electron_density = 1.69196288524929195294666892502269929e+24L,
        },
        {
            .name = "Neodymium",
            .total_atom_density = 2.88077589315031828077219125329447799e+22L,
            .electron_density = 1.72846553589019096846331475197668679e+24L,
        },
        {
            .name = "Promethium",
            .total_atom_density = 3.00041607911705413567086569833855392e+22L,
            .electron_density = 1.83025380826140302275922807598651789e+24L,
        },
        {
            .name = "Samarium",
            .total_atom_density = 2.98771424160972607346566563022713783e+22L,
            .electron_density = 1.85238282979803016554871269074082545e+24L,
        },
        {
            .name = "Europium",
            .total_atom_density = 2.07772930806147030004271777732517596e+22L,
            .electron_density = 1.30896946407872628902691219971486086e+24L,
        },
        {
            .name = "Gadolinium",
            .total_atom_density = 3.02539063236262833108039038137534330e+22L,
            .electron_density = 1.93625000471208213189144984408021971e+24L,
        },
        {
            .name = "Terbium",
            .total_atom_density = 3.11820581839757452444892084023389930e+22L,
            .electron_density = 2.02683378195842344089179854615203455e+24L,
        },
        {
            .name = "Dysprosium",
            .total_atom_density = 3.16858282018542978660090648321297512e+22L,
            .electron_density = 2.09126466132238365915659827892056358e+24L,
        },
        {
            .name = "Holmium",
            .total_atom_density = 3.21133949180053461059290138555350549e+22L,
            .electron_density = 2.15159745950635818909724392832084868e+24L,
        },
        {
            .name = "Erbium",
            .total_atom_density = 3.26420106460624290166702204538203766e+22L,
            .electron_density = 2.21965672393224517313357499085978561e+24L,
        },
        {
            .name = "Thulium",
            .total_atom_density = 3.32273557790213265670570424728105318e+22L,
            .electron_density = 2.29268754875247153312693593062392670e+24L,
        },
        {
            .name = "Ytterbium",
            .total_atom_density = 2.34198411705396719699582572170346609e+22L,
            .electron_density = 1.63938888193777703789707800519242626e+24L,
        },
        {
            .name = "Lutetium",
            .total_atom_density = 3.38680591227349405040543164252255505e+22L,
            .electron_density = 2.40463219771418077578785646619101409e+24L,
        },
        {
            .name = "Hafnium",
            .total_atom_density = 4.49083686938823308420221171401568355e+22L,
            .electron_density = 3.23340254595952782062559243409129216e+24L,
        },
        {
            .name = "Tantalum",
            .total_atom_density = 5.54129976265900057931452186682373141e+22L,
            .electron_density = 4.04514882674107042289960096278132393e+24L,
        },
        {
            .name = "Tungsten",
            .total_atom_density = 6.32213835176221519555434395007069909e+22L,
            .electron_density = 4.67838238030403924471021452305231733e+24L,
        },
        {
            .name = "Rhenium",
            .total_atom_density = 6.79811165119132532786735697537312275e+22L,
            .electron_density = 5.09858373839349399590051773152984206e+24L,
        },
        {
            .name = "Osmium",
            .total_atom_density = 7.14521307967003755138615381016531266e+22L,
            .electron_density = 5.43036194054922853905347689572563762e+24L,
        },
        {
            .name = "Iridium",
            .total_atom_density = 7.02419974940336837485290040349796419e+22L,
            .electron_density = 5.40863380704059364863673331069343243e+24L,
        },
        {
            .name = "Platinum",
            .total_atom_density = 6.62148689609282933012045874433849206e+22L,
            .electron_density = 5.16475977895240687749395782058402381e+24L,
        },
        {
            .name = "Gold",
            .total_atom_density = 5.90698001697409659395907690212417432e+22L,
            .electron_density = 4.66651421340953630922767075267809771e+24L,
        },
        {
            .name = "Mercury",
            .total_atom_density = 4.06781385484150755666282601774839304e+22L,
            .electron_density = 3.25425108387320604533026081419871443e+24L,
        },
        {
            .name = "Thallium",
            .total_atom_density = 3.45328854338353241969135980130220956e+22L,
            .electron_density = 2.79716372014066125995000143905478974e+24L,
        },
        {
            .name = "Lead",
            .total_atom_density = 3.29853863542638863703786405676239760e+22L,
            .electron_density = 2.70480168104963868237104852654516603e+24L,
        },
        {
            .name = "Bismuth",
            .total_atom_density = 2.80877088567138855415004792171576694e+22L,
            .electron_density = 2.33127983510725249994453977502408656e+24L,
        },
        {
            .name = "Polonium",
            .total_atom_density = 2.68569715297662408088867440033947285e+22L,
            .electron_density = 2.25598560850036422794648649628515719e+24L,
        },
        {
            .name = "Astatine",
            .total_atom_density = 2.86786159418876130795802937799148014e+22L,
            .electron_density = 2.43768235506044711176432497129275812e+24L,
        },
        {
            .name = "Radon",
            .total_atom_density = 2.45911738396121106964931613711047026e+19L,
            .electron_density = 2.11484095020664151989841187791500443e+21L,
        },
        {
            .name = "Francium",
            .total_atom_density = 2.70027258985480417938680355013176670e+22L,
            .electron_density = 2.34923715317367963606651908861463703e+24L,
        },
        {
            .name = "Radium",
            .total_atom_density = 1.33218225375817797985326235886597389e+22L,
            .electron_density = 1.17232038330719662227087087580205703e+24L,
        },
        {
            .name = "Actinium",
            .total_atom_density = 2.67116937162339996174779967621279318e+22L,
            .electron_density = 2.37734074074482596595554171182938593e+24L,
        },
        {
            .name = "Thorium",
            .total_atom_density = 3.04172046508092686423936333182290581e+22L,
            .electron_density = 2.73754841857283417781542699864061523e+24L,
        },
        {
            .name = "Protactinium",
            .total_atom_density = 4.00631721715399258151113104609942479e+22L,
            .electron_density = 3.64574866761013324917512925195047655e+24L,
        },
        {
            .name = "Uranium",
            .total_atom_density = 4.79435744861999563573840404796191935e+22L,
            .electron_density = 4.41080885273039598487933172412496581e+24L,
        },
        {
            .name = "Air",
            .total_atom_density = 4.98825359953291694777413339143953166e+19L,
            .electron_density = 3.62245988165303505823956788117902301e+20L,
        },
        {
            .name = "Water",
            .total_atom_density = 1.00283629783758988205989146576040806e+23L,
            .electron_density = 3.34279374695350345930706276806729550e+23L,
        },
        {
            .name = "Adipose",
            .total_atom_density = 1.03480465169837793320354815923058204e+23L,
            .electron_density = 3.09413180540656174819636047171599222e+23L,
        },
        {
            .name = "Blood",
            .total_atom_density = 1.01859089831923584146104457824690478e+23L,
            .electron_density = 3.51084399058520307813829824059759722e+23L,
        },
        {
            .name = "BloodIodine5",
            .total_atom_density = 1.14540076688477700791195629421327319e+23L,
            .electron_density = 4.09107322866665370412148253099357483e+23L,
        },
        {
            .name = "BloodIodine10",
            .total_atom_density = 1.25220490747146107565928701083758220e+23L,
            .electron_density = 4.65467348093720277502806490381741397e+23L,
        },
        {
            .name = "BloodIodine15",
            .total_atom_density = 1.35121683961427425268078837229175458e+23L,
            .electron_density = 5.23579794602256430915715374783585291e+23L,
        },
        {
            .name = "BloodIodine20",
            .total_atom_density = 1.42728972603280220858516794531606458e+23L,
            .electron_density = 5.78206520072485681622837950061961515e+23L,
        },
        {
            .name = "Brain",
            .total_atom_density = 1.04020936849074923096472276785695127e+23L,
            .electron_density = 3.43774355917375919630923781217946421e+23L,
        },
        {
            .name = "Breast",
            .total_atom_density = 1.03228431242424047090596746686921788e+23L,
            .electron_density = 3.39045671504537582373498954171363162e+23L,
        },
        {
            .name = "Heart",
            .total_atom_density = 1.02439357314983191897192754068102239e+23L,
            .electron_density = 3.48402641021808235441384728532790371e+23L,
        },
        {
            .name = "Intestine",
            .total_atom_density = 1.01377751370716922570103062585114702e+23L,
            .electron_density = 3.42412781375533351453947009529656210e+23L,
        },
        {
            .name = "Kidney",
            .total_atom_density = 1.01752872018627076615374310523802849e+23L,
            .electron_density = 3.48088011825058026553420109751349176e+23L,
        },
        {
            .name = "Liver",
            .total_atom_density = 1.02182396379378402014206328805624476e+23L,
            .electron_density = 3.51067875637631753957529252314209307e+23L,
        },
        {
            .name = "Lung",
            .total_atom_density = 2.51051988893293725126202797513484911e+22L,
            .electron_density = 8.61919325395532795195710987452699173e+22L,
        },
        {
            .name = "RibBone",
            .total_atom_density = 9.94180800022945669760208689116973000e+22L,
            .electron_density = 5.95221511757982685200712123306433434e+23L,
        },
        {
            .name = "SpineBone",
            .total_atom_density = 1.02437311368409939647624426460379489e+23L,
            .electron_density = 4.52997224025571572961269095714740483e+23L,
        },
        {
            .name = "Spleen",
            .total_atom_density = 1.02474793901543908362973037124055493e+23L,
            .electron_density = 3.51403815195876439908829656434632875e+23L,
        },
        {
            .name = "CdTe",
            .total_atom_density = 3.11125106005235864172454447752684342e+22L,
            .electron_density = 1.55562488467041549439623905877373804e+24L,
        },
        {
            .name = "CsI",
            .total_atom_density = 2.09074804049346721658606050732973335e+22L,
            .electron_density = 1.12900395074498139015063165772132182e+24L,
        },
        {
            .name = "GaAs",
            .total_atom_density = 4.42153443554230197899843646484573642e+22L,
            .electron_density = 1.41489101956345530018935357228123301e+24L,
        },
        {
            .name = "GOS",
            .total_atom_density = 5.91772944182190883193575615510608071e+22L,
            .electron_density = 1.89365360876908392025745026666389966e+24L,
        },
        {
            .name = "LSO",
            .total_atom_density = 7.77616381884032834392172238861303272e+22L,
            .electron_density = 1.90703546848164940270438289598638818e+24L,
        },
        {
            .name = "NaI",
            .total_atom_density = 2.94597502831913106187572366736960772e+22L,
            .electron_density = 9.43602084766863537232750709764425700e+23L,
        },
    },
};

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInMaterialsTest, ExposesCanonicalNamesInRegistryOrder) {
  auto const available = builtins::GetAvailableMaterialNames();

  ASSERT_EQ(available.size(), 117U);
  EXPECT_EQ(available.front(), "Vacuum");

  // Vacuum occupies slot 0; the 92 elemental Materials follow in Z order.
  EXPECT_EQ(available[1U], "Hydrogen");
  EXPECT_EQ(available[13U], "Aluminum");
  EXPECT_EQ(available[53U], "Iodine");
  EXPECT_EQ(available[74U], "Tungsten");
  EXPECT_EQ(available[92U], "Uranium");

  constexpr std::size_t k_compound_offset{93U};
  for (std::size_t index = 0U; index < k_compound_names.size(); ++index) {
    EXPECT_EQ(available[k_compound_offset + index], k_compound_names[index]);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInMaterialsTest, BuildsEveryAvailableMaterial) {
  for (auto const name : builtins::GetAvailableMaterialNames()) {
    SCOPED_TRACE(name);
    EXPECT_NO_THROW(static_cast<void>(builtins::BuildBuiltInMaterial(name)));
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInMaterialsTest, BuildsVacuum) {
  auto const material = builtins::BuildBuiltInMaterial("Vacuum");

  EXPECT_EQ(material.GetName(), "Vacuum");
  EXPECT_EQ(material.GetDensity(), 0.0_g_cm3);
  EXPECT_TRUE(material.GetConstituents().empty());
  EXPECT_EQ(material.GetTotalAtomDensityPerCubicCentimeter(), 0.0L);
  EXPECT_EQ(material.GetElectronDensityPerCubicCentimeter(), 0.0L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInMaterialsTest, BuildsCommonMedia) {
  auto const water = builtins::BuildBuiltInMaterial("Water");
  EXPECT_EQ(water.GetDensity(), 1.000_g_cm3);
  ASSERT_EQ(water.GetConstituents().size(), 2U);
  EXPECT_EQ(water.GetConstituents()[0U].atomic_number, 1U);
  EXPECT_EQ(water.GetConstituents()[0U].mass_fraction, 0.111898L);
  EXPECT_EQ(water.GetConstituents()[1U].atomic_number, 8U);
  EXPECT_EQ(water.GetConstituents()[1U].mass_fraction, 0.888102L);

  auto const air = builtins::BuildBuiltInMaterial("Air");
  EXPECT_EQ(air.GetDensity(), 1.205e-3_g_cm3);
  ASSERT_EQ(air.GetConstituents().size(), 4U);
  EXPECT_EQ(air.GetConstituents()[0U].atomic_number, 6U);
  EXPECT_EQ(air.GetConstituents()[1U].atomic_number, 7U);
  EXPECT_EQ(air.GetConstituents()[2U].atomic_number, 8U);
  EXPECT_EQ(air.GetConstituents()[3U].atomic_number, 18U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInMaterialsTest, BuildsRepresentativeElementalMaterials) {
  struct ExpectedMaterial {
    std::string_view name;
    std::uint32_t atomic_number;
    Density density;
  };

  constexpr std::array expected{
      ExpectedMaterial{
          .name = "Aluminum", .atomic_number = 13U, .density = 2.699_g_cm3},
      ExpectedMaterial{
          .name = "Silicon", .atomic_number = 14U, .density = 2.330_g_cm3},
      ExpectedMaterial{
          .name = "Copper", .atomic_number = 29U, .density = 8.960_g_cm3},
      ExpectedMaterial{
          .name = "Tungsten", .atomic_number = 74U, .density = 19.30_g_cm3},
      ExpectedMaterial{
          .name = "Lead", .atomic_number = 82U, .density = 11.35_g_cm3},
      ExpectedMaterial{
          .name = "Uranium", .atomic_number = 92U, .density = 18.95_g_cm3},
  };

  for (auto const &expected_material : expected) {
    auto const material =
        builtins::BuildBuiltInMaterial(expected_material.name);

    SCOPED_TRACE(expected_material.name);

    EXPECT_EQ(material.GetName(), expected_material.name);
    EXPECT_EQ(material.GetDensity(), expected_material.density);

    auto const constituents = material.GetConstituents();
    ASSERT_EQ(constituents.size(), 1U);
    EXPECT_EQ(constituents.front().atomic_number,
              expected_material.atomic_number);
    EXPECT_EQ(constituents.front().mass_fraction, 1.0L);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInMaterialsTest, BuildsRepresentativeMedicalMaterials) {
  auto const brain = builtins::BuildBuiltInMaterial("Brain");
  EXPECT_EQ(brain.GetDensity(), 1.03_g_cm3);
  EXPECT_EQ(brain.GetConstituents().size(), 13U);

  auto const lung = builtins::BuildBuiltInMaterial("Lung");
  EXPECT_EQ(lung.GetDensity(), 0.26_g_cm3);
  EXPECT_EQ(lung.GetConstituents().size(), 9U);

  auto const lso = builtins::BuildBuiltInMaterial("LSO");
  EXPECT_EQ(lso.GetDensity(), 7.4_g_cm3);
  EXPECT_EQ(lso.GetConstituents().size(), 3U);

  auto const cdte = builtins::BuildBuiltInMaterial("CdTe");
  EXPECT_EQ(cdte.GetDensity(), 6.200_g_cm3);
  ASSERT_EQ(cdte.GetConstituents().size(), 2U);
  EXPECT_EQ(cdte.GetConstituents()[0U].atomic_number, 48U);
  EXPECT_EQ(cdte.GetConstituents()[0U].mass_fraction, 0.468358L);
  EXPECT_EQ(cdte.GetConstituents()[1U].atomic_number, 52U);
  EXPECT_EQ(cdte.GetConstituents()[1U].mass_fraction, 0.531642L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInMaterialsTest, RejectsUnknownOrInexactNames) {
  EXPECT_THROW(static_cast<void>(builtins::BuildBuiltInMaterial("water")),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(builtins::BuildBuiltInMaterial(" Water ")),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(builtins::BuildBuiltInMaterial("Aluminium")),
               ggems::core::GGEMSRecoverable);

  // NIST labels are source descriptions, not public GGEMS canonical names.
  EXPECT_THROW(static_cast<void>(builtins::BuildBuiltInMaterial(
                   "Brain, Grey/White Matter (ICRU-44)")),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(
      static_cast<void>(builtins::BuildBuiltInMaterial("Cadmium Telluride")),
      ggems::core::GGEMSRecoverable);

  EXPECT_THROW(static_cast<void>(builtins::BuildBuiltInMaterial("Unknown")),
               ggems::core::GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInMaterialsTest, ResolvesEveryElementThroughIsotopeProfiles) {
  for (auto const name : builtins::GetAvailableMaterialNames()) {
    SCOPED_TRACE(name);

    auto const material = builtins::BuildBuiltInMaterial(name);

    if (name == "Vacuum") {
      EXPECT_TRUE(material.GetConstituents().empty());
      EXPECT_TRUE(material.GetIsotopeConstituents().empty());
      continue;
    }

    EXPECT_FALSE(material.GetIsotopeConstituents().empty());

    for (auto const &constituent : material.GetConstituents()) {
      bool const reference =
          std::ranges::find(k_reference_elements, constituent.atomic_number) !=
          k_reference_elements.end();

      EXPECT_EQ(constituent.isotope_profile,
                reference
                    ? materials::GGEMSIsotopeProfile::LegacyReferenceIsotope
                    : materials::GGEMSIsotopeProfile::Nist41Natural);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInMaterialsTest, MatchesIndependentIsotopicOracle) {
  // Independent Decimal oracle of the M2 migration (NIST 4.1 natural profiles
  // or legacy reference isotopes, AME2020/NUBASE2020 masses, CODATA 2022 M_u).
  constexpr long double k_relative_budget{
      64.0L * std::numeric_limits<long double>::epsilon(),
  };

  for (auto const &expected : k_expected_totals) {
    SCOPED_TRACE(expected.name);

    auto const material = builtins::BuildBuiltInMaterial(expected.name);

    EXPECT_LE(std::abs(material.GetTotalAtomDensityPerCubicCentimeter() -
                       expected.total_atom_density),
              k_relative_budget * expected.total_atom_density);
    EXPECT_LE(std::abs(material.GetElectronDensityPerCubicCentimeter() -
                       expected.electron_density),
              k_relative_budget * expected.electron_density);
  }
}
