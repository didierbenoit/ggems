import argparse
import csv
import json
from collections import Counter, defaultdict
from collections.abc import Iterable
from dataclasses import dataclass
from pathlib import Path
from typing import Protocol, cast

# ------------------------------------------------------------------------------

DIEHARDER_REFERENCE_VERSION = "3.31.1"
EXPECTED_ASSESSMENT_COUNT = 114
EXPECTED_PRIMARY_ASSESSMENT_COUNT = 106
ENGINE_ORDER = ("Philox", "PCG32", "JKISS")
ASSESSMENT_ORDER = ("PASSED", "WEAK", "FAILED")
CASE_OUTCOME_ORDER = ("passed", "weak", "failed")

type JsonObject = dict[str, object]
type CsvValue = str | int | float | bool | None
type CsvRow = dict[str, CsvValue]

# ------------------------------------------------------------------------------


@dataclass(frozen=True, slots=True)
class ExpectedCase:
    series: str
    seed_index: int
    engine: str
    seed: int
    stream_offset: int
    layout: str


# ------------------------------------------------------------------------------

EXPECTED_CASES: dict[str, ExpectedCase] = {
    "A01-philox": ExpectedCase("A", 1, "Philox", 741_750_298, 0, "interleaved"),
    "A02-pcg32": ExpectedCase("A", 2, "PCG32", 741_750_298, 0, "interleaved"),
    "A03-jkiss": ExpectedCase("A", 3, "JKISS", 741_750_298, 0, "interleaved"),
    "A04-philox": ExpectedCase("A", 4, "Philox", 3_392_895_365, 0, "interleaved"),
    "A05-pcg32": ExpectedCase("A", 5, "PCG32", 3_392_895_365, 0, "interleaved"),
    "A06-jkiss": ExpectedCase("A", 6, "JKISS", 3_392_895_365, 0, "interleaved"),
    "A07-philox": ExpectedCase("A", 7, "Philox", 2_374_528_848, 0, "interleaved"),
    "A08-pcg32": ExpectedCase("A", 8, "PCG32", 2_374_528_848, 0, "interleaved"),
    "A09-jkiss": ExpectedCase("A", 9, "JKISS", 2_374_528_848, 0, "interleaved"),
    "E01-jkiss": ExpectedCase("E", 1, "JKISS", 741_750_298, 0, "worker_major"),
    "E06-jkiss": ExpectedCase("E", 6, "JKISS", 3_392_895_365, 0, "worker_major"),
    "E12-jkiss": ExpectedCase("E", 12, "JKISS", 2_374_528_848, 0, "worker_major"),
    "F01-jkiss": ExpectedCase("F", 1, "JKISS", 741_750_298, 1_048_576, "interleaved"),
}

# ------------------------------------------------------------------------------


@dataclass(frozen=True, slots=True)
class CaseRecord:
    case_id: str
    series: str
    seed_index: int
    engine: str
    seed: int
    stream_type: str
    sample_bits: int
    layout: str
    stream_offset: int
    worker_count: int
    samples_per_worker: int
    byte_count: int
    status: str
    outcome: str
    assessment_count: int
    passed_count: int
    weak_count: int
    failed_count: int
    primary_passed_count: int
    primary_weak_count: int
    primary_failed_count: int
    elapsed_seconds: float | None
    input_file_offset_bytes: int | None
    remaining_input_bytes: int | None
    dieharder_version: str
    tool_return_code: int
    gsl_library: str
    package_versions: str
    device_name: str
    device_vendor: str
    driver_version: str


# ------------------------------------------------------------------------------


@dataclass(frozen=True, slots=True)
class AnomalyRecord:
    case_id: str
    series: str
    engine: str
    seed: int
    layout: str
    stream_offset: int
    test_id: int
    test_name: str
    ntuple: int
    tsamples: int
    psamples: int
    p_value: float
    assessment: str
    reliability: str
    implementation_caveat: bool
    primary: bool


# ------------------------------------------------------------------------------


class Arguments(Protocol):
    campaign_dir: Path | None
    output_dir: Path | None
    top: int
    no_write: bool


# ------------------------------------------------------------------------------


def ParseArguments() -> Arguments:
    parser = argparse.ArgumentParser(
        description="Aggregate the fixed GGEMS Dieharder campaign results."
    )
    _ = parser.add_argument(
        "--campaign-dir",
        type=Path,
        help=(
            "Directory containing campaign summary JSON files and manifests/. "
            + "Defaults to validation/random/results/dieharder/campaign."
        ),
    )
    _ = parser.add_argument(
        "--output-dir",
        type=Path,
        help="Output directory. Defaults to <campaign-dir>/aggregate.",
    )
    _ = parser.add_argument(
        "--top",
        type=int,
        default=15,
        help="Number of recurrent primary anomaly tests to print (default: 15).",
    )
    _ = parser.add_argument(
        "--no-write",
        action="store_true",
        help="Print the report without writing JSON or CSV files.",
    )
    return cast(Arguments, cast(object, parser.parse_args()))


# ------------------------------------------------------------------------------


def ProjectRoot() -> Path:
    return Path(__file__).resolve().parents[3]


# ------------------------------------------------------------------------------


def ResolveCampaignDirectory(requested: Path | None) -> Path:
    if requested is not None:
        return requested.expanduser().resolve()

    return (
        ProjectRoot()
        / "validation"
        / "random"
        / "results"
        / "dieharder"
        / "campaign"
    ).resolve()


# ------------------------------------------------------------------------------


def LoadJson(path: Path) -> JsonObject:
    try:
        value = cast(object, json.loads(path.read_text(encoding="utf-8")))
    except (json.JSONDecodeError, OSError) as error:
        raise RuntimeError(f"Failed to read JSON file {path}: {error}") from error

    if not isinstance(value, dict):
        raise TypeError(f"Expected a JSON object in {path}.")

    return cast(JsonObject, value)


# ------------------------------------------------------------------------------


def RequireObject(section: JsonObject, key: str, context: str) -> JsonObject:
    value = section.get(key)
    if not isinstance(value, dict):
        raise TypeError(f"Expected object field '{key}' for {context}.")
    return cast(JsonObject, value)


# ------------------------------------------------------------------------------


def RequireList(section: JsonObject, key: str, context: str) -> list[object]:
    value = section.get(key)
    if not isinstance(value, list):
        raise TypeError(f"Expected array field '{key}' for {context}.")
    return cast(list[object], value)


# ------------------------------------------------------------------------------


def RequireString(section: JsonObject, key: str, context: str) -> str:
    value = section.get(key)
    if not isinstance(value, str):
        raise TypeError(f"Expected string field '{key}' for {context}.")
    return value


# ------------------------------------------------------------------------------


def RequireInt(section: JsonObject, key: str, context: str) -> int:
    value = section.get(key)
    if isinstance(value, bool) or not isinstance(value, int):
        raise TypeError(f"Expected integer field '{key}' for {context}.")
    return value


# ------------------------------------------------------------------------------


def RequireBool(section: JsonObject, key: str, context: str) -> bool:
    value = section.get(key)
    if not isinstance(value, bool):
        raise TypeError(f"Expected boolean field '{key}' for {context}.")
    return value


# ------------------------------------------------------------------------------


def RequireNumber(section: JsonObject, key: str, context: str) -> float:
    value = section.get(key)
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise TypeError(f"Expected numeric field '{key}' for {context}.")
    return float(value)


# ------------------------------------------------------------------------------


def OptionalNumber(section: JsonObject, key: str, context: str) -> float | None:
    value = section.get(key)
    if value is None:
        return None
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise TypeError(f"Expected optional numeric field '{key}' for {context}.")
    return float(value)


# ------------------------------------------------------------------------------


def OptionalInt(section: JsonObject, key: str, context: str) -> int | None:
    value = section.get(key)
    if value is None:
        return None
    if isinstance(value, bool) or not isinstance(value, int):
        raise TypeError(f"Expected optional integer field '{key}' for {context}.")
    return value


# ------------------------------------------------------------------------------


def RequireStringArray(section: JsonObject, key: str, context: str) -> tuple[str, ...]:
    values = RequireList(section, key, context)
    result: list[str] = []

    for value in values:
        if not isinstance(value, str):
            raise TypeError(f"Expected string values in '{key}' for {context}.")
        result.append(value)

    return tuple(result)


# ------------------------------------------------------------------------------


def ValidateExpectedCase(
    case_id: str,
    expected: ExpectedCase,
    summary_input: JsonObject,
) -> None:
    checks: tuple[tuple[str, object], ...] = (
        ("engine", expected.engine),
        ("seed", expected.seed),
        ("stream_offset", expected.stream_offset),
        ("layout", expected.layout),
        ("stream_type", "raw_uint32"),
        ("sample_bits", 32),
        ("worker_count", 1_048_576),
        ("samples_per_worker", 61_036),
        ("byte_count", 256_003_538_944),
    )

    for key, expected_value in checks:
        if summary_input.get(key) != expected_value:
            raise RuntimeError(
                f"Unexpected {key} for {case_id}: "
                + f"got {summary_input.get(key)!r}, expected {expected_value!r}."
            )


# ------------------------------------------------------------------------------


def ValidateSummaryManifestAgreement(
    case_id: str,
    summary_input: JsonObject,
    random: JsonObject,
) -> None:
    shared_keys = (
        "engine",
        "seed",
        "stream_offset",
        "stream_type",
        "sample_bits",
        "layout",
        "worker_count",
        "samples_per_worker",
        "total_samples",
        "byte_count",
    )

    for key in shared_keys:
        if summary_input.get(key) != random.get(key):
            raise RuntimeError(f"Summary/manifest mismatch for {case_id}: {key}.")


# ------------------------------------------------------------------------------


def CountAssessmentObjects(
    case_id: str,
    values: list[object],
) -> tuple[Counter[str], Counter[str]]:
    all_counts: Counter[str] = Counter()
    primary_counts: Counter[str] = Counter()

    for value in values:
        if not isinstance(value, dict):
            raise TypeError(f"Invalid assessment object for {case_id}.")

        assessment = cast(JsonObject, value)
        context = f"assessment in {case_id}"
        assessment_name = RequireString(assessment, "assessment", context)
        reliability = RequireString(assessment, "reliability", context)
        caveat = RequireBool(assessment, "implementation_caveat", context)

        if assessment_name not in ASSESSMENT_ORDER:
            raise RuntimeError(
                f"Unknown Dieharder assessment '{assessment_name}' for {case_id}."
            )

        if reliability not in {"Good", "Suspect", "Do Not Use"}:
            raise RuntimeError(
                f"Unknown Dieharder reliability '{reliability}' for {case_id}."
            )

        all_counts[assessment_name] += 1
        if reliability == "Good" and not caveat:
            primary_counts[assessment_name] += 1

    return all_counts, primary_counts


# ------------------------------------------------------------------------------


def OutcomeFromPrimaryCounts(primary_counts: Counter[str]) -> str:
    if primary_counts["FAILED"] > 0:
        return "failed"
    if primary_counts["WEAK"] > 0:
        return "weak"
    return "passed"


# ------------------------------------------------------------------------------


def BuildAnomalies(
    case: CaseRecord,
    values: list[object],
) -> list[AnomalyRecord]:
    anomalies: list[AnomalyRecord] = []

    for value in values:
        if not isinstance(value, dict):
            raise TypeError(f"Invalid assessment object for {case.case_id}.")

        assessment = cast(JsonObject, value)
        context = f"assessment in {case.case_id}"
        assessment_name = RequireString(assessment, "assessment", context)

        if assessment_name == "PASSED":
            continue

        reliability = RequireString(assessment, "reliability", context)
        caveat = RequireBool(assessment, "implementation_caveat", context)

        anomalies.append(
            AnomalyRecord(
                case_id=case.case_id,
                series=case.series,
                engine=case.engine,
                seed=case.seed,
                layout=case.layout,
                stream_offset=case.stream_offset,
                test_id=RequireInt(assessment, "test_id", context),
                test_name=RequireString(assessment, "test_name", context),
                ntuple=RequireInt(assessment, "ntuple", context),
                tsamples=RequireInt(assessment, "tsamples", context),
                psamples=RequireInt(assessment, "psamples", context),
                p_value=RequireNumber(assessment, "p_value", context),
                assessment=assessment_name,
                reliability=reliability,
                implementation_caveat=caveat,
                primary=(reliability == "Good" and not caveat),
            )
        )

    return anomalies


# ------------------------------------------------------------------------------


def LoadCase(
    summary_path: Path,
    manifests_dir: Path,
) -> tuple[CaseRecord, list[AnomalyRecord]]:
    case_id = summary_path.stem
    expected = EXPECTED_CASES.get(case_id)

    if expected is None:
        raise RuntimeError(f"Unexpected Dieharder campaign case: {case_id}.")

    manifest_path = manifests_dir / summary_path.name
    if not manifest_path.is_file():
        raise RuntimeError(f"Missing manifest for {case_id}: {manifest_path}")

    summary = LoadJson(summary_path)
    manifest = LoadJson(manifest_path)

    tool = RequireObject(summary, "tool", case_id)
    protocol = RequireObject(summary, "protocol", case_id)
    summary_input = RequireObject(summary, "input", case_id)
    environment = RequireObject(summary, "environment", case_id)
    output = RequireObject(summary, "output", case_id)
    random = RequireObject(manifest, "random", case_id)
    opencl = RequireObject(manifest, "opencl", case_id)

    ValidateSummaryManifestAgreement(case_id, summary_input, random)
    ValidateExpectedCase(case_id, expected, summary_input)

    dieharder_version = RequireString(tool, "version", case_id)
    if dieharder_version != DIEHARDER_REFERENCE_VERSION:
        raise RuntimeError(
            f"Unexpected Dieharder version for {case_id}: {dieharder_version}."
        )

    tool_return_code = RequireInt(tool, "return_code", case_id)
    if tool_return_code != 0:
        raise RuntimeError(f"Nonzero Dieharder return code for {case_id}.")

    if RequireInt(protocol, "generator_id", case_id) != 200:
        raise RuntimeError(f"Unexpected Dieharder generator for {case_id}.")

    if (
        RequireInt(protocol, "expected_assessment_count", case_id)
        != EXPECTED_ASSESSMENT_COUNT
    ):
        raise RuntimeError(f"Unexpected assessment protocol count for {case_id}.")

    status = RequireString(output, "status", case_id)
    if status != "completed":
        raise RuntimeError(f"Campaign case is not completed: {case_id}.")

    if RequireString(output, "stderr", case_id):
        raise RuntimeError(f"Campaign case contains non-empty stderr: {case_id}.")

    assessments = RequireList(output, "assessments", case_id)
    if len(assessments) != EXPECTED_ASSESSMENT_COUNT:
        raise RuntimeError(
            f"Expected {EXPECTED_ASSESSMENT_COUNT} assessments for {case_id}, "
            + f"got {len(assessments)}."
        )

    all_counts, primary_counts = CountAssessmentObjects(case_id, assessments)
    if sum(primary_counts.values()) != EXPECTED_PRIMARY_ASSESSMENT_COUNT:
        raise RuntimeError(
            f"Expected {EXPECTED_PRIMARY_ASSESSMENT_COUNT} primary assessments "
            + f"for {case_id}, got {sum(primary_counts.values())}."
        )

    assessment_counts = RequireObject(output, "assessment_counts", case_id)
    primary_summary_counts = RequireObject(
        output, "primary_assessment_counts", case_id
    )

    for assessment_name in ASSESSMENT_ORDER:
        if (
            RequireInt(assessment_counts, assessment_name, case_id)
            != all_counts[assessment_name]
        ):
            raise RuntimeError(
                f"Assessment count mismatch for {case_id}: {assessment_name}."
            )

        if (
            RequireInt(primary_summary_counts, assessment_name, case_id)
            != primary_counts[assessment_name]
        ):
            raise RuntimeError(
                f"Primary assessment count mismatch for {case_id}: "
                + assessment_name
                + "."
            )

    package_versions = RequireStringArray(environment, "package_versions", case_id)

    case = CaseRecord(
        case_id=case_id,
        series=expected.series,
        seed_index=expected.seed_index,
        engine=expected.engine,
        seed=expected.seed,
        stream_type=RequireString(summary_input, "stream_type", case_id),
        sample_bits=RequireInt(summary_input, "sample_bits", case_id),
        layout=expected.layout,
        stream_offset=expected.stream_offset,
        worker_count=RequireInt(summary_input, "worker_count", case_id),
        samples_per_worker=RequireInt(
            summary_input, "samples_per_worker", case_id
        ),
        byte_count=RequireInt(summary_input, "byte_count", case_id),
        status=status,
        outcome=OutcomeFromPrimaryCounts(primary_counts),
        assessment_count=len(assessments),
        passed_count=all_counts["PASSED"],
        weak_count=all_counts["WEAK"],
        failed_count=all_counts["FAILED"],
        primary_passed_count=primary_counts["PASSED"],
        primary_weak_count=primary_counts["WEAK"],
        primary_failed_count=primary_counts["FAILED"],
        elapsed_seconds=OptionalNumber(output, "elapsed_seconds", case_id),
        input_file_offset_bytes=OptionalInt(
            output, "input_file_offset_bytes", case_id
        ),
        remaining_input_bytes=OptionalInt(output, "remaining_input_bytes", case_id),
        dieharder_version=dieharder_version,
        tool_return_code=tool_return_code,
        gsl_library=RequireString(environment, "gsl_library", case_id),
        package_versions="; ".join(package_versions),
        device_name=RequireString(opencl, "device_name", case_id),
        device_vendor=RequireString(opencl, "device_vendor", case_id),
        driver_version=RequireString(opencl, "driver_version", case_id),
    )

    return case, BuildAnomalies(case, assessments)


# ------------------------------------------------------------------------------


def LoadCampaign(
    campaign_dir: Path,
) -> tuple[list[CaseRecord], list[AnomalyRecord]]:
    manifests_dir = campaign_dir / "manifests"
    if not campaign_dir.is_dir() or not manifests_dir.is_dir():
        raise RuntimeError(f"Invalid campaign directory: {campaign_dir}")

    cases: list[CaseRecord] = []
    anomalies: list[AnomalyRecord] = []

    for summary_path in sorted(campaign_dir.glob("*.json")):
        case, case_anomalies = LoadCase(summary_path, manifests_dir)
        cases.append(case)
        anomalies.extend(case_anomalies)

    return cases, anomalies


# ------------------------------------------------------------------------------


def ValidateCampaign(cases: list[CaseRecord]) -> None:
    expected_ids = set(EXPECTED_CASES)
    actual_ids = {case.case_id for case in cases}

    if actual_ids != expected_ids:
        missing = sorted(expected_ids - actual_ids)
        unexpected = sorted(actual_ids - expected_ids)
        raise RuntimeError(
            "Unexpected Dieharder campaign case set: "
            + f"missing={missing}, unexpected={unexpected}."
        )

    if len(cases) != len(expected_ids):
        raise RuntimeError("Campaign contains duplicate case identifiers.")

    if len({case.dieharder_version for case in cases}) != 1:
        raise RuntimeError("Campaign contains inconsistent Dieharder versions.")
    if any(case.tool_return_code != 0 for case in cases):
        raise RuntimeError("Campaign contains a nonzero Dieharder return code.")
    if len({case.worker_count for case in cases}) != 1:
        raise RuntimeError("Campaign contains inconsistent worker counts.")
    if len({case.samples_per_worker for case in cases}) != 1:
        raise RuntimeError("Campaign contains inconsistent sample depths.")
    if len({case.byte_count for case in cases}) != 1:
        raise RuntimeError("Campaign contains inconsistent byte counts.")
    if len({case.gsl_library for case in cases}) != 1:
        raise RuntimeError("Campaign contains inconsistent GSL libraries.")


# ------------------------------------------------------------------------------


def CountAssessments(
    cases: Iterable[CaseRecord],
    *,
    primary: bool,
) -> Counter[str]:
    counts: Counter[str] = Counter()

    for case in cases:
        if primary:
            counts["PASSED"] += case.primary_passed_count
            counts["WEAK"] += case.primary_weak_count
            counts["FAILED"] += case.primary_failed_count
        else:
            counts["PASSED"] += case.passed_count
            counts["WEAK"] += case.weak_count
            counts["FAILED"] += case.failed_count

    return counts


# ------------------------------------------------------------------------------


def CountCaseOutcomes(cases: Iterable[CaseRecord]) -> Counter[str]:
    return Counter(case.outcome for case in cases)


# ------------------------------------------------------------------------------


def PrintReport(
    cases: list[CaseRecord],
    anomalies: list[AnomalyRecord],
    top: int,
) -> None:
    sample = cases[0]
    all_counts = CountAssessments(cases, primary=False)
    primary_counts = CountAssessments(cases, primary=True)

    print("GGEMS Dieharder campaign aggregate")
    print()
    print(f"Cases              : {len(cases)}")
    print(f"Bytes per case     : {sample.byte_count}")
    print(f"GiB per case       : {sample.byte_count / 1024**3:.8f}")
    print(f"Workers per case   : {sample.worker_count}")
    print(f"Samples per worker : {sample.samples_per_worker}")
    print(f"Assessments/case   : {sample.assessment_count}")
    print(f"Primary/case       : {EXPECTED_PRIMARY_ASSESSMENT_COUNT}")
    print(f"Dieharder version  : {sample.dieharder_version}")
    print(f"GSL library        : {sample.gsl_library}")
    print(f"OpenCL devices     : {', '.join(sorted({c.device_name for c in cases}))}")

    print()
    print("Assessment totals")
    print()
    print("Scope       PASSED    WEAK  FAILED")
    print("----------------------------------")
    print(
        f"{'All':<10} {all_counts['PASSED']:>7} "
        + f"{all_counts['WEAK']:>7} {all_counts['FAILED']:>7}"
    )
    print(
        f"{'Primary':<10} {primary_counts['PASSED']:>7} "
        + f"{primary_counts['WEAK']:>7} {primary_counts['FAILED']:>7}"
    )

    print()
    print("Primary assessments by engine")
    print()
    print("Engine   Cases  PASSED   WEAK  FAILED")
    print("-------------------------------------")
    for engine in ENGINE_ORDER:
        selected = [case for case in cases if case.engine == engine]
        counts = CountAssessments(selected, primary=True)
        print(
            f"{engine:<8} {len(selected):>5} {counts['PASSED']:>7} "
            + f"{counts['WEAK']:>6} {counts['FAILED']:>7}"
        )

    print()
    print("Primary case outcomes by engine")
    print()
    print("Engine   Cases  Passed  Weak  Failed")
    print("-----------------------------------")
    for engine in ENGINE_ORDER:
        selected = [case for case in cases if case.engine == engine]
        counts = CountCaseOutcomes(selected)
        print(
            f"{engine:<8} {len(selected):>5} {counts['passed']:>7} "
            + f"{counts['weak']:>5} {counts['failed']:>7}"
        )

    print()
    print("Primary assessments by engine and layout")
    print()
    print("Engine   Layout         Cases  PASSED   WEAK  FAILED")
    print("----------------------------------------------------")
    layout_keys = sorted(
        {(case.engine, case.layout) for case in cases},
        key=lambda item: (ENGINE_ORDER.index(item[0]), item[1]),
    )
    for engine, layout in layout_keys:
        selected = [
            case
            for case in cases
            if case.engine == engine and case.layout == layout
        ]
        counts = CountAssessments(selected, primary=True)
        print(
            f"{engine:<8} {layout:<14} {len(selected):>5} "
            + f"{counts['PASSED']:>7} {counts['WEAK']:>6} {counts['FAILED']:>7}"
        )

    failed_primary = [
        anomaly
        for anomaly in anomalies
        if anomaly.primary and anomaly.assessment == "FAILED"
    ]

    print()
    print("Primary FAILED assessments")
    print()
    if failed_primary:
        for anomaly in failed_primary:
            print(
                f"{anomaly.case_id:<11} {anomaly.engine:<6} "
                + f"{anomaly.layout:<12} id={anomaly.test_id:<3} "
                + f"{anomaly.test_name:<24} p={anomaly.p_value:.8g}"
            )
    else:
        print("None")

    if top > 0:
        primary_anomalies = [anomaly for anomaly in anomalies if anomaly.primary]
        line_counts = Counter(
            (
                anomaly.engine,
                anomaly.test_id,
                anomaly.test_name,
                anomaly.assessment,
            )
            for anomaly in primary_anomalies
        )
        case_sets: defaultdict[tuple[str, int, str, str], set[str]] = defaultdict(set)
        layout_sets: defaultdict[tuple[str, int, str, str], set[str]] = defaultdict(set)

        for anomaly in primary_anomalies:
            key = (
                anomaly.engine,
                anomaly.test_id,
                anomaly.test_name,
                anomaly.assessment,
            )
            case_sets[key].add(anomaly.case_id)
            layout_sets[key].add(anomaly.layout)

        ranked = sorted(
            case_sets,
            key=lambda key: (len(case_sets[key]), line_counts[key], key),
            reverse=True,
        )[:top]

        print()
        print("Most recurrent primary anomaly tests")
        print()
        print("Engine   A  ID   Test                     Cases Lines Layouts")
        print("-----------------------------------------------------------")
        for key in ranked:
            engine, test_id, test_name, assessment = key
            print(
                f"{engine:<8} {assessment[0]:<1} {test_id:<4} {test_name:<24} "
                + f"{len(case_sets[key]):>5} {line_counts[key]:>5} "
                + ",".join(sorted(layout_sets[key]))
            )

    elapsed_values = [
        case.elapsed_seconds for case in cases if case.elapsed_seconds is not None
    ]
    remaining_values = [
        case.remaining_input_bytes
        for case in cases
        if case.remaining_input_bytes is not None
    ]

    if elapsed_values:
        print()
        print("Execution metrics")
        print()
        print(f"Timed cases         : {len(elapsed_values)}")
        print(f"Mean elapsed        : {sum(elapsed_values) / len(elapsed_values):.3f} s")
        print(f"Min elapsed         : {min(elapsed_values):.3f} s")
        print(f"Max elapsed         : {max(elapsed_values):.3f} s")

    if remaining_values:
        print(f"Measured margins    : {len(remaining_values)}")
        print(
            "Mean remaining      : "
            + f"{sum(remaining_values) / len(remaining_values):.0f} bytes"
        )
        print(f"Min remaining       : {min(remaining_values)} bytes")
        print(f"Max remaining       : {max(remaining_values)} bytes")


# ------------------------------------------------------------------------------


def CountsToJson(counts: Counter[str], order: tuple[str, ...]) -> JsonObject:
    return {key: counts[key] for key in order}


# ------------------------------------------------------------------------------


def CaseToJson(case: CaseRecord) -> JsonObject:
    return {
        "case_id": case.case_id,
        "series": case.series,
        "seed_index": case.seed_index,
        "engine": case.engine,
        "seed": case.seed,
        "stream_type": case.stream_type,
        "sample_bits": case.sample_bits,
        "layout": case.layout,
        "stream_offset": case.stream_offset,
        "worker_count": case.worker_count,
        "samples_per_worker": case.samples_per_worker,
        "byte_count": case.byte_count,
        "status": case.status,
        "outcome": case.outcome,
        "assessment_count": case.assessment_count,
        "passed_count": case.passed_count,
        "weak_count": case.weak_count,
        "failed_count": case.failed_count,
        "primary_passed_count": case.primary_passed_count,
        "primary_weak_count": case.primary_weak_count,
        "primary_failed_count": case.primary_failed_count,
        "elapsed_seconds": case.elapsed_seconds,
        "input_file_offset_bytes": case.input_file_offset_bytes,
        "remaining_input_bytes": case.remaining_input_bytes,
        "dieharder_version": case.dieharder_version,
        "tool_return_code": case.tool_return_code,
        "gsl_library": case.gsl_library,
        "package_versions": case.package_versions,
        "device_name": case.device_name,
        "device_vendor": case.device_vendor,
        "driver_version": case.driver_version,
    }


# ------------------------------------------------------------------------------


def AnomalyToJson(anomaly: AnomalyRecord) -> JsonObject:
    return {
        "case_id": anomaly.case_id,
        "series": anomaly.series,
        "engine": anomaly.engine,
        "seed": anomaly.seed,
        "layout": anomaly.layout,
        "stream_offset": anomaly.stream_offset,
        "test_id": anomaly.test_id,
        "test_name": anomaly.test_name,
        "ntuple": anomaly.ntuple,
        "tsamples": anomaly.tsamples,
        "psamples": anomaly.psamples,
        "p_value": anomaly.p_value,
        "assessment": anomaly.assessment,
        "reliability": anomaly.reliability,
        "implementation_caveat": anomaly.implementation_caveat,
        "primary": anomaly.primary,
    }


# ------------------------------------------------------------------------------


def BuildAggregate(cases: list[CaseRecord], anomalies: list[AnomalyRecord]) -> JsonObject:
    sample = cases[0]
    by_engine: JsonObject = {}

    for engine in ENGINE_ORDER:
        selected = [case for case in cases if case.engine == engine]
        by_engine[engine] = {
            "case_count": len(selected),
            "case_outcomes": CountsToJson(
                CountCaseOutcomes(selected), CASE_OUTCOME_ORDER
            ),
            "assessments": CountsToJson(
                CountAssessments(selected, primary=False), ASSESSMENT_ORDER
            ),
            "primary_assessments": CountsToJson(
                CountAssessments(selected, primary=True), ASSESSMENT_ORDER
            ),
        }

    by_series_engine: JsonObject = {}
    for series in ("A", "E", "F"):
        series_data: JsonObject = {}
        for engine in ENGINE_ORDER:
            selected = [
                case
                for case in cases
                if case.series == series and case.engine == engine
            ]
            if selected:
                series_data[engine] = {
                    "case_count": len(selected),
                    "case_outcomes": CountsToJson(
                        CountCaseOutcomes(selected), CASE_OUTCOME_ORDER
                    ),
                    "primary_assessments": CountsToJson(
                        CountAssessments(selected, primary=True), ASSESSMENT_ORDER
                    ),
                }
        by_series_engine[series] = series_data

    by_engine_layout: JsonObject = {}
    for engine, layout in sorted(
        {(case.engine, case.layout) for case in cases},
        key=lambda item: (ENGINE_ORDER.index(item[0]), item[1]),
    ):
        selected = [
            case
            for case in cases
            if case.engine == engine and case.layout == layout
        ]
        by_engine_layout[f"{engine}/{layout}"] = {
            "case_count": len(selected),
            "case_outcomes": CountsToJson(
                CountCaseOutcomes(selected), CASE_OUTCOME_ORDER
            ),
            "primary_assessments": CountsToJson(
                CountAssessments(selected, primary=True), ASSESSMENT_ORDER
            ),
        }

    primary_anomalies = [anomaly for anomaly in anomalies if anomaly.primary]
    line_counts = Counter(
        (
            anomaly.engine,
            anomaly.test_id,
            anomaly.test_name,
            anomaly.assessment,
        )
        for anomaly in primary_anomalies
    )
    case_sets: defaultdict[tuple[str, int, str, str], set[str]] = defaultdict(set)
    layout_sets: defaultdict[tuple[str, int, str, str], set[str]] = defaultdict(set)

    for anomaly in primary_anomalies:
        key = (
            anomaly.engine,
            anomaly.test_id,
            anomaly.test_name,
            anomaly.assessment,
        )
        case_sets[key].add(anomaly.case_id)
        layout_sets[key].add(anomaly.layout)

    recurrent_primary_anomalies: list[object] = []
    for key in sorted(
        case_sets,
        key=lambda item: (len(case_sets[item]), line_counts[item], item),
        reverse=True,
    ):
        engine, test_id, test_name, assessment = key
        recurrent_primary_anomalies.append(
            {
                "engine": engine,
                "test_id": test_id,
                "test_name": test_name,
                "assessment": assessment,
                "case_count": len(case_sets[key]),
                "line_count": line_counts[key],
                "case_ids": sorted(case_sets[key]),
                "layouts": sorted(layout_sets[key]),
            }
        )

    elapsed_values = [
        case.elapsed_seconds for case in cases if case.elapsed_seconds is not None
    ]
    remaining_values = [
        case.remaining_input_bytes
        for case in cases
        if case.remaining_input_bytes is not None
    ]

    runtime: JsonObject = {
        "elapsed_case_count": len(elapsed_values),
        "remaining_margin_case_count": len(remaining_values),
    }
    if elapsed_values:
        runtime["elapsed_seconds_total"] = sum(elapsed_values)
        runtime["elapsed_seconds_mean"] = sum(elapsed_values) / len(elapsed_values)
        runtime["elapsed_seconds_min"] = min(elapsed_values)
        runtime["elapsed_seconds_max"] = max(elapsed_values)
    if remaining_values:
        runtime["remaining_input_bytes_mean"] = (
            sum(remaining_values) / len(remaining_values)
        )
        runtime["remaining_input_bytes_min"] = min(remaining_values)
        runtime["remaining_input_bytes_max"] = max(remaining_values)

    return {
        "schema_version": 1,
        "campaign": {
            "case_count": len(cases),
            "byte_count_per_case": sample.byte_count,
            "worker_count_per_case": sample.worker_count,
            "samples_per_worker": sample.samples_per_worker,
            "assessment_count_per_case": sample.assessment_count,
            "primary_assessment_count_per_case": EXPECTED_PRIMARY_ASSESSMENT_COUNT,
            "dieharder_version": sample.dieharder_version,
            "gsl_library": sample.gsl_library,
            "package_versions": sorted({case.package_versions for case in cases}),
            "devices": sorted({case.device_name for case in cases}),
        },
        "assessment_totals": CountsToJson(
            CountAssessments(cases, primary=False), ASSESSMENT_ORDER
        ),
        "primary_assessment_totals": CountsToJson(
            CountAssessments(cases, primary=True), ASSESSMENT_ORDER
        ),
        "by_engine": by_engine,
        "by_series_engine": by_series_engine,
        "by_engine_layout": by_engine_layout,
        "recurrent_primary_anomalies": recurrent_primary_anomalies,
        "failed_primary_cases": [
            CaseToJson(case) for case in cases if case.primary_failed_count > 0
        ],
        "failed_primary_assessments": [
            AnomalyToJson(anomaly)
            for anomaly in anomalies
            if anomaly.primary and anomaly.assessment == "FAILED"
        ],
        "runtime": runtime,
    }


# ------------------------------------------------------------------------------


def CaseToRow(case: CaseRecord) -> CsvRow:
    return cast(CsvRow, CaseToJson(case))


# ------------------------------------------------------------------------------


def AnomalyToRow(anomaly: AnomalyRecord) -> CsvRow:
    return cast(CsvRow, AnomalyToJson(anomaly))


# ------------------------------------------------------------------------------


def WriteCsv(path: Path, rows: list[CsvRow]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if not rows:
        _ = path.write_text("", encoding="utf-8")
        return

    with path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0].keys()))
        _ = writer.writeheader()
        for row in rows:
            _ = writer.writerow(row)


# ------------------------------------------------------------------------------


def WriteOutputs(
    output_dir: Path,
    cases: list[CaseRecord],
    anomalies: list[AnomalyRecord],
) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    summary_path = output_dir / "campaign_summary.json"
    cases_path = output_dir / "cases.csv"
    anomalies_path = output_dir / "anomalies.csv"

    _ = summary_path.write_text(
        json.dumps(BuildAggregate(cases, anomalies), indent=2) + "\n",
        encoding="utf-8",
    )
    WriteCsv(cases_path, [CaseToRow(case) for case in cases])
    WriteCsv(anomalies_path, [AnomalyToRow(anomaly) for anomaly in anomalies])

    print()
    print("Aggregate outputs")
    print()
    print(f"Summary   : {summary_path}")
    print(f"Cases CSV : {cases_path}")
    print(f"Anomalies : {anomalies_path}")


# ------------------------------------------------------------------------------


def main() -> int:
    args = ParseArguments()
    campaign_dir = ResolveCampaignDirectory(args.campaign_dir)
    output_dir = (
        args.output_dir.expanduser().resolve()
        if args.output_dir is not None
        else campaign_dir / "aggregate"
    )

    cases, anomalies = LoadCampaign(campaign_dir)
    ValidateCampaign(cases)
    PrintReport(cases, anomalies, args.top)

    if not args.no_write:
        WriteOutputs(output_dir, cases, anomalies)

    return 0


# ------------------------------------------------------------------------------

if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, RuntimeError, TypeError, ValueError) as error:
        print(f"GGEMS Dieharder campaign aggregation failed:\n{error}")
        raise SystemExit(1) from None
