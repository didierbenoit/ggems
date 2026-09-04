import argparse
import csv
import json
import re
from collections import Counter, defaultdict
from collections.abc import Iterable
from dataclasses import dataclass
from pathlib import Path
from typing import Protocol, cast

EXPECTED_SERIES_CASES = {"A": 36, "B": 36, "C": 9, "D": 9, "E": 9, "F": 3}
EXPECTED_CASE_COUNT = sum(EXPECTED_SERIES_CASES.values())
ENGINE_ORDER = ("Philox", "PCG32", "JKISS")
STATUS_ORDER = (
    "passed_no_anomalies",
    "attention_required",
    "failed",
    "tool_error",
)
SEVERITY_ORDER = (
    "unusual",
    "mildly_suspicious",
    "suspicious",
    "very_suspicious",
    "fail",
)
CASE_ID_PATTERN = re.compile(r"^([A-F])(\d{2})-(philox|pcg32|jkiss)$")
LOW_BITS_PATTERN = re.compile(r"^\[(Low\d+/\d+)\]")
TEST_NAME_PATTERN = re.compile(r"^(.*?)\s+R=")


@dataclass(frozen=True, slots=True)
class CaseRecord:
    case_id: str
    series: str
    seed_index: int
    engine: str
    seed: int
    stream_type: str
    sample_bits: int
    lanes_used: int | None
    layout: str
    stream_offset: int
    worker_count: int
    samples_per_worker: int
    byte_count: int
    status: str
    tested_length: str
    test_result_count: int | None
    anomaly_count: int
    practrand_input: str
    practrand_version: str
    max_size: str
    test_set: str
    folding: str
    multithreaded: bool
    tool_return_code: int
    device_name: str
    device_vendor: str
    driver_version: str


@dataclass(frozen=True, slots=True)
class AnomalyRecord:
    case_id: str
    series: str
    engine: str
    seed: int
    stream_type: str
    lanes_used: int | None
    layout: str
    stream_offset: int
    status: str
    tested_length: str
    severity: str
    bit_view: str
    test_name: str
    test_family: str
    raw_line: str


class Arguments(Protocol):
    campaign_dir: Path | None
    output_dir: Path | None
    top: int
    no_write: bool


type JsonObject = dict[str, object]
type CsvValue = str | int | bool | None
type CsvRow = dict[str, CsvValue]


def ParseArguments() -> Arguments:
    parser = argparse.ArgumentParser(
        description="Aggregate the fixed GGEMS PractRand validation campaign."
    )
    _ = parser.add_argument(
        "--campaign-dir",
        type=Path,
        help=(
            "Campaign directory containing summary JSON files and manifests/. "
            "Defaults to validation/random/results/practrand/campaign."
        ),
    )
    _ = parser.add_argument(
        "--output-dir",
        type=Path,
        help="Aggregate output directory. Defaults to <campaign-dir>/aggregate.",
    )
    _ = parser.add_argument(
        "--top",
        type=int,
        default=15,
        help="Number of recurring anomaly families to print (default: 15).",
    )
    _ = parser.add_argument(
        "--no-write",
        action="store_true",
        help="Print the report without writing JSON or CSV files.",
    )
    return cast(Arguments, cast(object, parser.parse_args()))


def ProjectRoot() -> Path:
    return Path(__file__).resolve().parents[3]


def ResolveCampaignDirectory(requested: Path | None) -> Path:
    if requested is not None:
        return requested.expanduser().resolve()

    return (
        ProjectRoot() / "validation" / "random" / "results" / "practrand" / "campaign"
    ).resolve()


def LoadJson(path: Path) -> JsonObject:
    try:
        value = cast(
            object,
            json.loads(path.read_text(encoding="utf-8")),
        )
    except (json.JSONDecodeError, OSError) as error:
        raise RuntimeError(f"Failed to read JSON file {path}: {error}") from error

    if not isinstance(value, dict):
        raise TypeError(f"Expected a JSON object in {path}.")

    return cast(JsonObject, value)


def RequireObject(section: JsonObject, key: str, context: str) -> JsonObject:
    value = section.get(key)
    if not isinstance(value, dict):
        raise TypeError(f"Expected object field '{key}' for {context}.")

    return cast(JsonObject, value)


def RequireString(section: JsonObject, key: str, context: str) -> str:
    value = section.get(key)
    if not isinstance(value, str):
        raise TypeError(f"Expected string field '{key}' for {context}.")

    return value


def RequireInteger(section: JsonObject, key: str, context: str) -> int:
    value = section.get(key)
    if not isinstance(value, int) or isinstance(value, bool):
        raise TypeError(f"Expected integer field '{key}' for {context}.")

    return value


def RequireBoolean(section: JsonObject, key: str, context: str) -> bool:
    value = section.get(key)
    if not isinstance(value, bool):
        raise TypeError(f"Expected boolean field '{key}' for {context}.")

    return value


def OptionalInteger(section: JsonObject, key: str, context: str) -> int | None:
    if key not in section:
        return None

    return RequireInteger(section, key, context)


def NullableInteger(section: JsonObject, key: str, context: str) -> int | None:
    value = section.get(key)
    if value is None:
        return None
    if not isinstance(value, int) or isinstance(value, bool):
        raise TypeError(f"Expected integer or null field '{key}' for {context}.")

    return value


def RequireStringList(section: JsonObject, key: str, context: str) -> list[str]:
    value = section.get(key)
    if not isinstance(value, list):
        raise TypeError(f"Expected string array field '{key}' for {context}.")

    items = cast(list[object], value)
    if not all(isinstance(item, str) for item in items):
        raise TypeError(f"Expected string array field '{key}' for {context}.")

    return cast(list[str], items)


def ParseCaseId(case_id: str) -> tuple[str, int]:
    match = CASE_ID_PATTERN.fullmatch(case_id)
    if match is None:
        raise RuntimeError(f"Invalid campaign case identifier: {case_id}")
    return match.group(1), int(match.group(2))


def ParseSeverity(line: str) -> str:
    lowered = line.lower()
    if "fail" in lowered:
        return "fail"
    if "very suspicious" in lowered:
        return "very_suspicious"
    if "mildly suspicious" in lowered:
        return "mildly_suspicious"
    if "suspicious" in lowered:
        return "suspicious"
    if "unusual" in lowered:
        return "unusual"
    return "unknown"


def ParseTestName(line: str) -> str:
    match = TEST_NAME_PATTERN.match(line)
    if match is not None:
        return match.group(1).strip()

    marker = line.find("R=")
    return line[:marker].strip() if marker >= 0 else line.strip()


def ParseBitView(test_name: str) -> str:
    match = LOW_BITS_PATTERN.match(test_name)
    return match.group(1) if match is not None else "full"


def ParseTestFamily(test_name: str) -> str:
    name = LOW_BITS_PATTERN.sub("", test_name).strip()
    delimiters = [
        position for position in (name.find("("), name.find(":")) if position > 0
    ]
    return name[: min(delimiters)] if delimiters else name


def LoadCase(
    summary_path: Path, manifests_dir: Path
) -> tuple[CaseRecord, list[AnomalyRecord]]:
    case_id = summary_path.stem
    series, seed_index = ParseCaseId(case_id)
    manifest_path = manifests_dir / summary_path.name

    if not manifest_path.is_file():
        raise RuntimeError(f"Missing manifest for {case_id}: {manifest_path}")

    summary = LoadJson(summary_path)
    manifest = LoadJson(manifest_path)

    tool = RequireObject(summary, "tool", case_id)
    summary_input = RequireObject(summary, "input", case_id)
    output = RequireObject(summary, "output", case_id)
    random = RequireObject(manifest, "random", case_id)
    opencl = RequireObject(manifest, "opencl", case_id)

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

    anomaly_lines = RequireStringList(output, "anomalies", case_id)

    case = CaseRecord(
        case_id=case_id,
        series=series,
        seed_index=seed_index,
        engine=RequireString(summary_input, "engine", case_id),
        seed=RequireInteger(summary_input, "seed", case_id),
        stream_type=RequireString(summary_input, "stream_type", case_id),
        sample_bits=RequireInteger(summary_input, "sample_bits", case_id),
        lanes_used=OptionalInteger(random, "lanes_used", case_id),
        layout=RequireString(summary_input, "layout", case_id),
        stream_offset=RequireInteger(summary_input, "stream_offset", case_id),
        worker_count=RequireInteger(summary_input, "worker_count", case_id),
        samples_per_worker=RequireInteger(summary_input, "samples_per_worker", case_id),
        byte_count=RequireInteger(summary_input, "byte_count", case_id),
        status=RequireString(output, "status", case_id),
        tested_length=RequireString(output, "tested_length", case_id),
        test_result_count=NullableInteger(output, "test_result_count", case_id),
        anomaly_count=len(anomaly_lines),
        practrand_input=RequireString(summary_input, "practrand_input", case_id),
        practrand_version=RequireString(tool, "version", case_id),
        max_size=RequireString(tool, "max_size", case_id),
        test_set=RequireString(tool, "test_set", case_id),
        folding=RequireString(tool, "folding", case_id),
        multithreaded=RequireBoolean(tool, "multithreaded", case_id),
        tool_return_code=RequireInteger(tool, "return_code", case_id),
        device_name=RequireString(opencl, "device_name", case_id),
        device_vendor=RequireString(opencl, "device_vendor", case_id),
        driver_version=RequireString(opencl, "driver_version", case_id),
    )

    if case.status not in STATUS_ORDER:
        raise RuntimeError(f"Unknown PractRand status '{case.status}' for {case_id}.")

    anomalies: list[AnomalyRecord] = []
    for line in anomaly_lines:
        test_name = ParseTestName(line)
        anomalies.append(
            AnomalyRecord(
                case_id=case.case_id,
                series=case.series,
                engine=case.engine,
                seed=case.seed,
                stream_type=case.stream_type,
                lanes_used=case.lanes_used,
                layout=case.layout,
                stream_offset=case.stream_offset,
                status=case.status,
                tested_length=case.tested_length,
                severity=ParseSeverity(line),
                bit_view=ParseBitView(test_name),
                test_name=test_name,
                test_family=ParseTestFamily(test_name),
                raw_line=line,
            )
        )

    return case, anomalies


def LoadCampaign(campaign_dir: Path) -> tuple[list[CaseRecord], list[AnomalyRecord]]:
    manifests_dir = campaign_dir / "manifests"
    if not campaign_dir.is_dir() or not manifests_dir.is_dir():
        raise RuntimeError(f"Invalid campaign directory: {campaign_dir}")

    summary_paths = sorted(campaign_dir.glob("*.json"))
    cases: list[CaseRecord] = []
    anomalies: list[AnomalyRecord] = []

    for summary_path in summary_paths:
        case, case_anomalies = LoadCase(summary_path, manifests_dir)
        cases.append(case)
        anomalies.extend(case_anomalies)

    return cases, anomalies


def ValidateCampaign(cases: list[CaseRecord]) -> None:
    if len(cases) != EXPECTED_CASE_COUNT:
        raise RuntimeError(f"Expected {EXPECTED_CASE_COUNT} cases, got {len(cases)}.")

    case_ids = [case.case_id for case in cases]
    if len(case_ids) != len(set(case_ids)):
        raise RuntimeError("Campaign contains duplicate case identifiers.")

    series_counts = Counter(case.series for case in cases)
    if dict(series_counts) != EXPECTED_SERIES_CASES:
        raise RuntimeError(f"Unexpected series counts: {dict(series_counts)}")

    common_tool_configs = {
        (case.practrand_version, case.max_size, case.test_set, case.multithreaded)
        for case in cases
    }
    if len(common_tool_configs) != 1:
        raise RuntimeError("Campaign contains inconsistent PractRand configurations.")

    folding_by_input: defaultdict[str, set[str]] = defaultdict(set)
    for case in cases:
        folding_by_input[case.practrand_input].add(case.folding)
    if any(len(values) != 1 for values in folding_by_input.values()):
        raise RuntimeError("Campaign contains inconsistent folding for one input mode.")

    if any(case.tool_return_code != 0 for case in cases):
        raise RuntimeError("Campaign contains a nonzero PractRand tool return code.")
    if len({case.worker_count for case in cases}) != 1:
        raise RuntimeError("Campaign contains inconsistent worker counts.")
    if len({case.byte_count for case in cases}) != 1:
        raise RuntimeError("Campaign contains inconsistent byte counts.")


def CountStatuses(cases: Iterable[CaseRecord]) -> Counter[str]:
    return Counter(case.status for case in cases)


def PrintReport(
    cases: list[CaseRecord], anomalies: list[AnomalyRecord], top: int
) -> None:
    sample = cases[0]
    input_configs = sorted({(case.practrand_input, case.folding) for case in cases})

    print("GGEMS PractRand campaign aggregate")
    print()
    print(f"Cases              : {len(cases)}")
    print(f"Bytes per case     : {sample.byte_count}")
    print(f"GiB per case       : {sample.byte_count / 1024**3:.8f}")
    print(f"Workers per case   : {sample.worker_count}")
    print(f"PractRand version  : {sample.practrand_version}")
    print(f"Test set           : {sample.test_set}")
    print(f"Max size           : {sample.max_size}")
    print(f"Multithreaded      : {'yes' if sample.multithreaded else 'no'}")
    print(f"OpenCL devices     : {', '.join(sorted({c.device_name for c in cases}))}")
    for practrand_input, folding in input_configs:
        print(f"Input configuration: {practrand_input} / {folding}")

    print()
    print("Status by engine")
    print()
    print("Engine   Cases  Clean  Attention  Failed  ToolErr  Anomaly lines")
    print("----------------------------------------------------------------")
    for engine in ENGINE_ORDER:
        selected = [case for case in cases if case.engine == engine]
        counts = CountStatuses(selected)
        anomaly_count = sum(1 for anomaly in anomalies if anomaly.engine == engine)
        print(
            f"{engine:<8} {len(selected):>5} "
            + f"{counts['passed_no_anomalies']:>6} "
            + f"{counts['attention_required']:>10} "
            + f"{counts['failed']:>7} "
            + f"{counts['tool_error']:>8} {anomaly_count:>14}"
        )

    print()
    print("Status by series and engine")
    print()
    print("Series  Engine   Cases  Clean  Attention  Failed  ToolErr")
    print("----------------------------------------------------------")
    for series in EXPECTED_SERIES_CASES:
        for engine in ENGINE_ORDER:
            selected = [
                case
                for case in cases
                if case.series == series and case.engine == engine
            ]
            if not selected:
                continue
            counts = CountStatuses(selected)
            print(
                f"{series:<6}  {engine:<8} {len(selected):>5} "
                + f"{counts['passed_no_anomalies']:>6} "
                + f"{counts['attention_required']:>10} "
                + f"{counts['failed']:>7} {counts['tool_error']:>8}"
            )

    print()
    print("Anomaly severity lines by engine")
    print()
    print("Engine   Unusual  MildSusp  Suspicious  VerySusp  FAIL  Unknown")
    print("----------------------------------------------------------------")
    for engine in ENGINE_ORDER:
        counts = Counter(
            anomaly.severity for anomaly in anomalies if anomaly.engine == engine
        )
        print(
            f"{engine:<8} {counts['unusual']:>7} "
            + f"{counts['mildly_suspicious']:>9} "
            + f"{counts['suspicious']:>10} "
            + f"{counts['very_suspicious']:>9} "
            + f"{counts['fail']:>5} {counts['unknown']:>8}"
        )

    fail_lines = [anomaly for anomaly in anomalies if anomaly.severity == "fail"]
    if fail_lines:
        fail_line_counts = Counter((a.engine, a.bit_view) for a in fail_lines)
        fail_case_sets: defaultdict[tuple[str, str], set[str]] = defaultdict(set)
        for anomaly in fail_lines:
            fail_case_sets[(anomaly.engine, anomaly.bit_view)].add(anomaly.case_id)

        print()
        print("FAIL distribution by bit view")
        print()
        print("Engine   View       Cases  Lines")
        print("--------------------------------")
        for key in sorted(
            fail_case_sets,
            key=lambda item: (ENGINE_ORDER.index(item[0]), item[1]),
        ):
            engine, bit_view = key
            print(
                f"{engine:<8} {bit_view:<10} "
                + f"{len(fail_case_sets[key]):>5} {fail_line_counts[key]:>6}"
            )

    failed_cases = [case for case in cases if case.status == "failed"]
    print()
    print("Failed cases")
    print()
    if failed_cases:
        for case in failed_cases:
            print(
                f"{case.case_id:<12} {case.engine:<6} seed={case.seed:<10} "
                + f"{case.tested_length}"
            )
    else:
        print("None")

    if top > 0:
        family_lines = Counter((a.engine, a.test_family, a.bit_view) for a in anomalies)
        family_cases: defaultdict[tuple[str, str, str], set[str]] = defaultdict(set)
        for anomaly in anomalies:
            family_cases[(anomaly.engine, anomaly.test_family, anomaly.bit_view)].add(
                anomaly.case_id
            )

        ranked = sorted(
            family_cases,
            key=lambda key: (len(family_cases[key]), family_lines[key], key),
            reverse=True,
        )[:top]

        print()
        print("Most recurrent anomaly families")
        print()
        print("Engine   View       Family                 Cases  Lines")
        print("-------------------------------------------------------")
        for key in ranked:
            engine, family, bit_view = key
            print(
                f"{engine:<8} {bit_view:<10} {family:<22} "
                + f"{len(family_cases[key]):>5} {family_lines[key]:>6}"
            )


def CaseToRow(case: CaseRecord) -> CsvRow:
    return {
        "case_id": case.case_id,
        "series": case.series,
        "seed_index": case.seed_index,
        "engine": case.engine,
        "seed": case.seed,
        "stream_type": case.stream_type,
        "sample_bits": case.sample_bits,
        "lanes_used": case.lanes_used,
        "layout": case.layout,
        "stream_offset": case.stream_offset,
        "worker_count": case.worker_count,
        "samples_per_worker": case.samples_per_worker,
        "byte_count": case.byte_count,
        "status": case.status,
        "tested_length": case.tested_length,
        "test_result_count": case.test_result_count,
        "anomaly_count": case.anomaly_count,
        "practrand_input": case.practrand_input,
        "practrand_version": case.practrand_version,
        "max_size": case.max_size,
        "test_set": case.test_set,
        "folding": case.folding,
        "multithreaded": case.multithreaded,
        "tool_return_code": case.tool_return_code,
        "device_name": case.device_name,
        "device_vendor": case.device_vendor,
        "driver_version": case.driver_version,
    }


def AnomalyToRow(anomaly: AnomalyRecord) -> CsvRow:
    return {
        "case_id": anomaly.case_id,
        "series": anomaly.series,
        "engine": anomaly.engine,
        "seed": anomaly.seed,
        "stream_type": anomaly.stream_type,
        "lanes_used": anomaly.lanes_used,
        "layout": anomaly.layout,
        "stream_offset": anomaly.stream_offset,
        "status": anomaly.status,
        "tested_length": anomaly.tested_length,
        "severity": anomaly.severity,
        "bit_view": anomaly.bit_view,
        "test_name": anomaly.test_name,
        "test_family": anomaly.test_family,
        "raw_line": anomaly.raw_line,
    }


def BuildAggregate(
    cases: list[CaseRecord], anomalies: list[AnomalyRecord]
) -> JsonObject:
    status_by_engine: dict[str, dict[str, int]] = {}
    for engine in ENGINE_ORDER:
        counts = CountStatuses(case for case in cases if case.engine == engine)
        status_by_engine[engine] = {status: counts[status] for status in STATUS_ORDER}

    status_by_series_engine: dict[str, dict[str, dict[str, int]]] = {}
    for series in EXPECTED_SERIES_CASES:
        status_by_series_engine[series] = {}
        for engine in ENGINE_ORDER:
            selected = [
                case
                for case in cases
                if case.series == series and case.engine == engine
            ]
            if selected:
                counts = CountStatuses(selected)
                status_by_series_engine[series][engine] = {
                    status: counts[status] for status in STATUS_ORDER
                }

    severity_by_engine: dict[str, dict[str, int]] = {}
    for engine in ENGINE_ORDER:
        counts = Counter(
            anomaly.severity for anomaly in anomalies if anomaly.engine == engine
        )
        severity_by_engine[engine] = {
            severity: counts[severity] for severity in (*SEVERITY_ORDER, "unknown")
        }

    family_lines = Counter((a.engine, a.test_family, a.bit_view) for a in anomalies)
    family_cases: defaultdict[tuple[str, str, str], set[str]] = defaultdict(set)
    for anomaly in anomalies:
        family_cases[(anomaly.engine, anomaly.test_family, anomaly.bit_view)].add(
            anomaly.case_id
        )

    recurrent_families: list[JsonObject] = [
        {
            "engine": key[0],
            "test_family": key[1],
            "bit_view": key[2],
            "case_count": len(family_cases[key]),
            "line_count": family_lines[key],
        }
        for key in sorted(
            family_cases,
            key=lambda item: (len(family_cases[item]), family_lines[item], item),
            reverse=True,
        )
    ]

    input_configurations: list[JsonObject] = [
        {"practrand_input": practrand_input, "folding": folding}
        for practrand_input, folding in sorted(
            {(case.practrand_input, case.folding) for case in cases}
        )
    ]

    campaign: JsonObject = {
        "case_count": len(cases),
        "byte_count_per_case": cases[0].byte_count,
        "worker_count_per_case": cases[0].worker_count,
        "practrand_version": cases[0].practrand_version,
        "max_size": cases[0].max_size,
        "test_set": cases[0].test_set,
        "multithreaded": cases[0].multithreaded,
        "input_configurations": input_configurations,
        "devices": sorted({case.device_name for case in cases}),
    }

    return {
        "schema_version": 1,
        "campaign": campaign,
        "status_by_engine": status_by_engine,
        "status_by_series_engine": status_by_series_engine,
        "severity_by_engine": severity_by_engine,
        "recurrent_anomaly_families": recurrent_families,
        "failed_cases": [CaseToRow(case) for case in cases if case.status == "failed"],
    }


def WriteCsv(path: Path, rows: list[CsvRow]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if not rows:
        _ = path.write_text("", encoding="utf-8")
        return

    with path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0].keys()))
        writer.writeheader()
        writer.writerows(rows)


def WriteOutputs(
    output_dir: Path, cases: list[CaseRecord], anomalies: list[AnomalyRecord]
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


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, RuntimeError, TypeError, ValueError) as error:
        print(f"GGEMS PractRand campaign aggregation failed:\n{error}")
        raise SystemExit(1) from None
