$materials = @(
    "Air_J"
    "Lung_J",
    "Muscle_J",
    "SpineBone_J",
    "RibBone_J",
    "Water"
)

$energies = @(
    "1.00000E-03",
    "1.50000E-03",
    "2.00000E-03",
    "3.00000E-03",
    "4.00000E-03",
    "5.00000E-03",
    "6.00000E-03",
    "8.00000E-03",
    "1.00000E-02",
    "1.50000E-02",
    "2.00000E-02",
    "3.00000E-02",
    "4.00000E-02",
    "5.00000E-02",
    "6.00000E-02",
    "8.00000E-02",
    "1.00000E-01",
    "1.50000E-01",
    "2.00000E-01",
    "3.00000E-01",
    "4.00000E-01",
    "5.00000E-01",
    "6.00000E-01",
    "8.00000E-01",
    "1.00000E+00"
)

$results = @()

foreach ($material in $materials) {
    Write-Host ""
    Write-Host "========================================"
    Write-Host "Material: $material"
    Write-Host "========================================"

    foreach ($energy in $energies) {
        Write-Host "  -> Energy: $energy MeV"

        $output = python .\cross_sections.py -d 0 -m $material -p Compton -e $energy 2>&1
        $text = $output -join "`n"

        $attenuation_match = [regex]::Match(
            $text,
            '(?m)^\s*Attenuation:\s*([0-9eE+\-\.]+)\s*cm-1'
        )

        $energy_attenuation_match = [regex]::Match(
            $text,
            '(?m)^\s*Energy attenuation:\s*([0-9eE+\-\.]+)\s*cm-1'
        )

        $cross_section_match = [regex]::Match(
            $text,
            '(?im)cross section is\s*([0-9eE+\-\.]+)\s*cm2\.g-1'
        )

        $attenuation = $null
        $energy_attenuation = $null
        $cross_section = $null

        if ($attenuation_match.Success) {
            $attenuation = [double]$attenuation_match.Groups[1].Value
        }

        if ($energy_attenuation_match.Success) {
            $energy_attenuation = [double]$energy_attenuation_match.Groups[1].Value
        }

        if ($cross_section_match.Success) {
            $cross_section = [double]$cross_section_match.Groups[1].Value
        }

        $results += [pscustomobject]@{
            Material                = $material
            Energy_MeV              = [double]$energy
            CrossSection_cm2_g      = $cross_section
            Attenuation_cm_1        = $attenuation
            EnergyAttenuation_cm_1  = $energy_attenuation
        }
    }
}

$results | Export-Csv -Path .\cross_sections_results.csv -NoTypeInformation -Encoding utf8
Write-Host ""
Write-Host "CSV generated: .\cross_sections_results.csv"
