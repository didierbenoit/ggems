#include <string>

#include <gtest/gtest.h>

#include "GGEMS/materials/GGEMSMaterialDescription.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"

namespace {

namespace materials = ggems::core::materials;
namespace builtins = ggems::core::materials::builtins;

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialDescriptionTest, DescribesMaterial) {
  auto const material = builtins::BuildBuiltInMaterial("Water");

  auto const description = materials::DescribeMaterial(material);

  EXPECT_NE(description.find("Material: Water"), std::string::npos);
  EXPECT_NE(description.find("Hydrogen"), std::string::npos);
  EXPECT_NE(description.find("Oxygen"), std::string::npos);
  EXPECT_NE(description.find("Atom density"), std::string::npos);
  EXPECT_NE(description.find("Electron density"), std::string::npos);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialDescriptionTest, DescribesVacuum) {
  auto const material = builtins::BuildBuiltInMaterial("Vacuum");

  auto const description = materials::DescribeMaterial(material);

  EXPECT_NE(description.find("Material: Vacuum"), std::string::npos);
  EXPECT_NE(description.find("Constituents     : 0"), std::string::npos);
}

// =============================================================================
// =============================================================================

TEST(GGEMSMaterialDescriptionTest, DescribesAvailableMaterials) {
  auto const description = materials::DescribeAvailableMaterials();

  EXPECT_NE(description.find("Available built-in Materials: 117"),
            std::string::npos);
  EXPECT_NE(description.find("Vacuum"), std::string::npos);
  EXPECT_NE(description.find("Hydrogen"), std::string::npos);
  EXPECT_NE(description.find("Uranium"), std::string::npos);
  EXPECT_NE(description.find("Brain"), std::string::npos);
  EXPECT_NE(description.find("LSO"), std::string::npos);
  EXPECT_NE(description.find("CdTe"), std::string::npos);
}
