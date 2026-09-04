import argparse
import csv
import json
import re
from collections import Counter, defaultdict
from collections.abc import Iterable
from dataclasses import dataclass
from pathlib import Path
from typing import Protocol, cast

# ------------------------------------------------------------------------------

EXPECTED_CASE_COUNT = 81
EXPECTED_SERIES_CASES = {"A": 51, "E": 15, "F": 15}
EXPECTED_BATTERY_CASES = {"smallcrush": 60, "crush": 18, "bigcrush": 3}
EXPECTED_SLOT_COUNTS = {"smallcrush": 15, "crush": 144, "bigcrush": 160}

ENGINE_ORDER = ("Philox", "PCG32", "JKISS")
BATTERY_ORDER = ("SmallCrush", "Crush", "BigCrush")
STATUS_ORDER = (
    "statistically_complete_no_testu01_suspects",
    "statistically_complete_with_testu01_suspects",
    "result_invalid_or_incomplete",
    "technical_error",
)
CASE_OUTCOME_ORDER = ("clean", "suspect", "incomplete", "technical_error")
CLASSIFICATION_ORDER = ("suspect_low", "suspect_high")

ENGINE_DISPLAY_NAMES = {
    "philox": "Philox",
    "pcg32": "PCG32",
    "jkiss": "JKISS",
}
BATTERY_DISPLAY_NAMES = {
    "smallcrush": "SmallCrush",
    "crush": "Crush",
    "bigcrush": "BigCrush",
}

CASE_ID_PATTERN = re.compile(
    r"^([AEF])(\d{2})-(philox|pcg32|jkiss)-(smallcrush|crush|bigcrush)$"
)

type JsonObject = dict[str, object]
type CsvValue = str | int | float | bool | None
type CsvRow = dict[str, CsvValue]

# ------------------------------------------------------------------------------


@dataclass(frozen=True, slots=True)
class CaseRecord:
    case_id: str
    series: str
    index: int
    engine: str
    battery: str
    battery_key: str
    seed: int
    stream_offset: int
    layout: str
    worker_count: int
    samples_per_worker: int
    logical_word_capacity: int
    logical_byte_capacity: int
    max_chunk_mib: int
    local_size: int
    device_selector: str
    status: str
    outcome: str
    consumed_word_count: int | None
    consumed_byte_count: int | None
    input_bytes_read: int | None
    reported_slot_count: int | None
    observed_slot_count: int
    suspect_low_count: int
    suspect_high_count: int
    producer_elapsed_seconds: float | None
    consumer_elapsed_seconds: float | None


@dataclass(frozen=True, slots=True)
class AnomalyRecord:
    case_id: str
    series: str
    engine: str
    battery: str
    seed: int
    layout: str
    stream_offset: int
    slot_index: int
    test_name: str
    p_value: float
    p_value_hex: str
    p_value_decimal: str
    classification: str


class Arguments(Protocol):
    campaign_dir: Path | None
    output_dir: Path | None
    top: int
    no_write: bool


# ------------------------------------------------------------------------------


def ParseArguments() -> Arguments:
    parser = argparse.ArgumentParser(
        description="Aggregate the fixed GGEMS TestU01 validation campaign."
    )
    _ = parser.add_argument(
        "--campaign-dir",
        type=Path,
        help=(
            "Campaign directory containing summary JSON files and requests/. "
            "Defaults to validation/random/results/testu01/campaign."
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
        help="Number of recurrent suspect tests to print (default: 15).",
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


def ResolveCampaignDirectory(requested: Path | None) -> Path:
    if requested is not None:
        return requested.expanduser().resolve()

    return (
        ProjectRoot() / "validation" / "random" / "results" / "testu01" / "campaign"
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


def RequireObject(section: JsonObject, key: str, context: str) -> JsonObject:
    value = section.get(key)
    if not isinstance(value, dict):
        raise TypeError(f"Expected object field '{key}' for {context}.")
    return cast(JsonObject, value)


def RequireList(section: JsonObject, key: str, context: str) -> list[object]:
    value = section.get(key)
    if not isinstance(value, list):
        raise TypeError(f"Expected array field '{key}' for {context}.")
    return cast(list[object], value)


def RequireString(section: JsonObject, key: str, context: str) -> str:
    value = section.get(key)
    if not isinstance(value, str):
        raise TypeError(f"Expected string field '{key}' for {context}.")
    return value


def RequireInt(section: JsonObject, key: str, context: str) -> int:
    value = section.get(key)
    if isinstance(value, bool) or not isinstance(value, int):
        raise TypeError(f"Expected integer field '{key}' for {context}.")
    return value


def RequireNumber(section: JsonObject, key: str, context: str) -> float:
    value = section.get(key)
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise TypeError(f"Expected numeric field '{key}' for {context}.")
    return float(value)


def OptionalInt(section: JsonObject, key: str, context: str) -> int | None:
    value = section.get(key)
    if value is None:
        return None
    if isinstance(value, bool) or not isinstance(value, int):
        raise TypeError(f"Expected optional integer field '{key}' for {context}.")
    return value


def OptionalNumber(section: JsonObject, key: str, context: str) -> float | None:
    value = section.get(key)
    if value is None:
        return None
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise TypeError(f"Expected optional numeric field '{key}' for {context}.")
    return float(value)


# ------------------------------------------------------------------------------


def ParseCaseId(case_id: str) -> tuple[str, int, str, str]:
    match = CASE_ID_PATTERN.fullmatch(case_id)
    if match is None:
        raise RuntimeError(f"Invalid TestU01 campaign case identifier: {case_id}")
    return match.group(1), int(match.group(2)), match.group(3), match.group(4)


def OutcomeFromStatus(status: str) -> str:
    if status == "statistically_complete_no_testu01_suspects":
        return "clean"
    if status == "statistically_complete_with_testu01_suspects":
        return "suspect"
    if status == "result_invalid_or_incomplete":
        return "incomplete"
    if status == "technical_error":
        return "technical_error"
    raise RuntimeError(f"Unknown TestU01 status: {status}")


# ------------------------------------------------------------------------------


def ValidateSummaryRequestAgreement(
    case_id: str,
    summary_input: JsonObject,
    request: JsonObject,
) -> None:
    random = RequireObject(request, "random", case_id)
    opencl = RequireObject(request, "opencl", case_id)

    shared = (
        ("engine", random.get("engine")),
        ("seed", random.get("seed")),
        ("stream_offset", random.get("stream_offset")),
        ("layout", random.get("layout")),
        ("worker_count", random.get("worker_count")),
        ("samples_per_worker", random.get("samples_per_worker")),
        ("logical_word_capacity", random.get("logical_word_capacity")),
        ("logical_byte_capacity", random.get("logical_byte_capacity")),
        ("max_chunk_mib", opencl.get("max_chunk_mib")),
        ("local_size", opencl.get("local_size")),
        ("device_selector", opencl.get("device_selector")),
    )

    for key, expected in shared:
        if summary_input.get(key) != expected:
            raise RuntimeError(f"Summary/request mismatch for {case_id}: {key}.")


# ------------------------------------------------------------------------------


def BuildAnomalies(case: CaseRecord, results: list[object]) -> list[AnomalyRecord]:
    anomalies: list[AnomalyRecord] = []

    for value in results:
        if not isinstance(value, dict):
            raise TypeError(f"Invalid TestU01 result slot for {case.case_id}.")
        slot = cast(JsonObject, value)
        classification = RequireString(slot, "classification", case.case_id)

        if classification not in CLASSIFICATION_ORDER:
            continue

        anomalies.append(
            AnomalyRecord(
                case_id=case.case_id,
                series=case.series,
                engine=case.engine,
                battery=case.battery,
                seed=case.seed,
                layout=case.layout,
                stream_offset=case.stream_offset,
                slot_index=RequireInt(slot, "index", case.case_id),
                test_name=RequireString(slot, "name", case.case_id),
                p_value=RequireNumber(slot, "p_value", case.case_id),
                p_value_hex=RequireString(slot, "p_value_hex", case.case_id),
                p_value_decimal=RequireString(slot, "p_value_decimal", case.case_id),
                classification=classification,
            )
        )

    return anomalies


def LoadCase(
    summary_path: Path,
    requests_dir: Path,
) -> tuple[CaseRecord, list[AnomalyRecord]]:
    case_id = summary_path.stem
    series, index, engine_key, battery_key = ParseCaseId(case_id)

    request_path = requests_dir / summary_path.name
    if not request_path.is_file():
        raise RuntimeError(f"Missing TestU01 request for {case_id}: {request_path}")

    summary = LoadJson(summary_path)
    request = LoadJson(request_path)

    tool = RequireObject(summary, "tool", case_id)
    summary_input = RequireObject(summary, "input", case_id)
    consumption = RequireObject(summary, "consumption", case_id)
    output = RequireObject(summary, "output", case_id)

    if RequireString(tool, "name", case_id) != "TestU01":
        raise RuntimeError(f"Unexpected tool name for {case_id}.")

    battery_from_summary = RequireString(tool, "battery", case_id)
    if battery_from_summary != battery_key:
        raise RuntimeError(f"Battery mismatch for {case_id}.")

    engine = ENGINE_DISPLAY_NAMES[engine_key]
    battery = BATTERY_DISPLAY_NAMES[battery_key]
    expected_slots = EXPECTED_SLOT_COUNTS[battery_key]

    if RequireString(summary_input, "engine", case_id) != engine_key:
        raise RuntimeError(f"Engine mismatch for {case_id}.")
    if summary_input.get("case_id") != case_id:
        raise RuntimeError(f"Case identifier mismatch for {case_id}.")

    ValidateSummaryRequestAgreement(case_id, summary_input, request)

    status = RequireString(output, "status", case_id)
    if status not in STATUS_ORDER:
        raise RuntimeError(f"Unknown TestU01 status '{status}' for {case_id}.")

    results = RequireList(output, "results", case_id)
    observed_slot_count = RequireInt(output, "observed_slot_count", case_id)
    if observed_slot_count != len(results):
        raise RuntimeError(f"Observed slot count mismatch for {case_id}.")

    if (
        status.startswith("statistically_complete_")
        and observed_slot_count != expected_slots
    ):
        raise RuntimeError(
            f"Expected {expected_slots} TestU01 slots for completed case {case_id}, "
            + f"got {observed_slot_count}."
        )

    producer_section = summary.get("producer")
    consumer_section = summary.get("consumer")
    producer_elapsed = (
        OptionalNumber(cast(JsonObject, producer_section), "elapsed_seconds", case_id)
        if isinstance(producer_section, dict)
        else None
    )
    consumer_elapsed = (
        OptionalNumber(cast(JsonObject, consumer_section), "elapsed_seconds", case_id)
        if isinstance(consumer_section, dict)
        else None
    )

    case = CaseRecord(
        case_id=case_id,
        series=series,
        index=index,
        engine=engine,
        battery=battery,
        battery_key=battery_key,
        seed=RequireInt(summary_input, "seed", case_id),
        stream_offset=RequireInt(summary_input, "stream_offset", case_id),
        layout=RequireString(summary_input, "layout", case_id),
        worker_count=RequireInt(summary_input, "worker_count", case_id),
        samples_per_worker=RequireInt(summary_input, "samples_per_worker", case_id),
        logical_word_capacity=RequireInt(
            summary_input, "logical_word_capacity", case_id
        ),
        logical_byte_capacity=RequireInt(
            summary_input, "logical_byte_capacity", case_id
        ),
        max_chunk_mib=RequireInt(summary_input, "max_chunk_mib", case_id),
        local_size=RequireInt(summary_input, "local_size", case_id),
        device_selector=RequireString(summary_input, "device_selector", case_id),
        status=status,
        outcome=OutcomeFromStatus(status),
        consumed_word_count=OptionalInt(consumption, "word_count", case_id),
        consumed_byte_count=OptionalInt(consumption, "byte_count", case_id),
        input_bytes_read=OptionalInt(consumption, "input_bytes_read", case_id),
        reported_slot_count=OptionalInt(output, "reported_slot_count", case_id),
        observed_slot_count=observed_slot_count,
        suspect_low_count=RequireInt(output, "suspect_low_count", case_id),
        suspect_high_count=RequireInt(output, "suspect_high_count", case_id),
        producer_elapsed_seconds=producer_elapsed,
        consumer_elapsed_seconds=consumer_elapsed,
    )

    return case, BuildAnomalies(case, results)


# ------------------------------------------------------------------------------


def LoadCampaign(
    campaign_dir: Path,
) -> tuple[list[CaseRecord], list[AnomalyRecord]]:
    requests_dir = campaign_dir / "requests"
    if not campaign_dir.is_dir() or not requests_dir.is_dir():
        raise RuntimeError(f"Invalid campaign directory: {campaign_dir}")

    cases: list[CaseRecord] = []
    anomalies: list[AnomalyRecord] = []

    for summary_path in sorted(campaign_dir.glob("*.json")):
        case, case_anomalies = LoadCase(summary_path, requests_dir)
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

    battery_counts = Counter(case.battery_key for case in cases)
    if dict(battery_counts) != EXPECTED_BATTERY_CASES:
        raise RuntimeError(f"Unexpected battery counts: {dict(battery_counts)}")


# ------------------------------------------------------------------------------


def CountOutcomes(cases: Iterable[CaseRecord]) -> Counter[str]:
    return Counter(case.outcome for case in cases)


def CountSuspects(anomalies: Iterable[AnomalyRecord]) -> Counter[str]:
    return Counter(anomaly.classification for anomaly in anomalies)


# ------------------------------------------------------------------------------


def PrintReport(
    cases: list[CaseRecord],
    anomalies: list[AnomalyRecord],
    top: int,
) -> None:
    print("GGEMS TestU01 campaign aggregate")
    print()
    print(f"Cases              : {len(cases)}")
    print(f"SmallCrush         : {sum(c.battery_key == 'smallcrush' for c in cases)}")
    print(f"Crush              : {sum(c.battery_key == 'crush' for c in cases)}")
    print(f"BigCrush           : {sum(c.battery_key == 'bigcrush' for c in cases)}")
    print(
        f"OpenCL selectors   : {', '.join(sorted({c.device_selector for c in cases}))}"
    )

    print()
    print("Case outcomes by engine")
    print()
    print("Engine   Cases  Clean  Suspect  Incomplete  Technical")
    print("-----------------------------------------------------")
    for engine in ENGINE_ORDER:
        selected = [case for case in cases if case.engine == engine]
        counts = CountOutcomes(selected)
        print(
            f"{engine:<8} {len(selected):>5} "
            + f"{counts['clean']:>6} {counts['suspect']:>8} "
            + f"{counts['incomplete']:>10} {counts['technical_error']:>10}"
        )

    print()
    print("Case outcomes by battery and engine")
    print()
    print("Battery     Engine   Cases  Clean  Suspect  Incomplete  Technical")
    print("----------------------------------------------------------------")
    for battery in BATTERY_ORDER:
        for engine in ENGINE_ORDER:
            selected = [
                case
                for case in cases
                if case.battery == battery and case.engine == engine
            ]
            if not selected:
                continue
            counts = CountOutcomes(selected)
            print(
                f"{battery:<11} {engine:<8} {len(selected):>5} "
                + f"{counts['clean']:>6} {counts['suspect']:>8} "
                + f"{counts['incomplete']:>10} {counts['technical_error']:>10}"
            )

    print()
    print("Suspect slots by engine")
    print()
    print("Engine   Low  High  Total")
    print("-------------------------")
    for engine in ENGINE_ORDER:
        counts = CountSuspects(a for a in anomalies if a.engine == engine)
        print(
            f"{engine:<8} {counts['suspect_low']:>3} "
            + f"{counts['suspect_high']:>5} {sum(counts.values()):>6}"
        )

    problematic = [
        case for case in cases if case.outcome in {"incomplete", "technical_error"}
    ]
    print()
    print("Incomplete or technical cases")
    print()
    if problematic:
        for case in problematic:
            print(
                f"{case.case_id:<30} {case.engine:<6} {case.battery:<10} {case.outcome}"
            )
    else:
        print("None")

    if top > 0:
        line_counts = Counter(
            (a.engine, a.battery, a.test_name, a.classification) for a in anomalies
        )
        case_sets: defaultdict[tuple[str, str, str, str], set[str]] = defaultdict(set)
        for anomaly in anomalies:
            key = (
                anomaly.engine,
                anomaly.battery,
                anomaly.test_name,
                anomaly.classification,
            )
            case_sets[key].add(anomaly.case_id)

        ranked = sorted(
            case_sets,
            key=lambda key: (len(case_sets[key]), line_counts[key], key),
            reverse=True,
        )[:top]

        print()
        print("Most recurrent suspect tests")
        print()
        print("Engine   Battery     Side  Test                         Cases  Lines")
        print("------------------------------------------------------------------")
        for key in ranked:
            engine, battery, test_name, classification = key
            side = "low" if classification == "suspect_low" else "high"
            print(
                f"{engine:<8} {battery:<11} {side:<5} {test_name:<28} "
                + f"{len(case_sets[key]):>5} {line_counts[key]:>6}"
            )


# ------------------------------------------------------------------------------


def CountsToJson(counts: Counter[str], order: tuple[str, ...]) -> JsonObject:
    return {key: counts[key] for key in order}


def CaseToJson(case: CaseRecord) -> JsonObject:
    return {
        "case_id": case.case_id,
        "series": case.series,
        "index": case.index,
        "engine": case.engine,
        "battery": case.battery,
        "seed": case.seed,
        "stream_offset": case.stream_offset,
        "layout": case.layout,
        "worker_count": case.worker_count,
        "samples_per_worker": case.samples_per_worker,
        "logical_word_capacity": case.logical_word_capacity,
        "logical_byte_capacity": case.logical_byte_capacity,
        "max_chunk_mib": case.max_chunk_mib,
        "local_size": case.local_size,
        "device_selector": case.device_selector,
        "status": case.status,
        "outcome": case.outcome,
        "consumed_word_count": case.consumed_word_count,
        "consumed_byte_count": case.consumed_byte_count,
        "input_bytes_read": case.input_bytes_read,
        "reported_slot_count": case.reported_slot_count,
        "observed_slot_count": case.observed_slot_count,
        "suspect_low_count": case.suspect_low_count,
        "suspect_high_count": case.suspect_high_count,
        "producer_elapsed_seconds": case.producer_elapsed_seconds,
        "consumer_elapsed_seconds": case.consumer_elapsed_seconds,
    }


def AnomalyToJson(anomaly: AnomalyRecord) -> JsonObject:
    return {
        "case_id": anomaly.case_id,
        "series": anomaly.series,
        "engine": anomaly.engine,
        "battery": anomaly.battery,
        "seed": anomaly.seed,
        "layout": anomaly.layout,
        "stream_offset": anomaly.stream_offset,
        "slot_index": anomaly.slot_index,
        "test_name": anomaly.test_name,
        "p_value": anomaly.p_value,
        "p_value_hex": anomaly.p_value_hex,
        "p_value_decimal": anomaly.p_value_decimal,
        "classification": anomaly.classification,
    }


def BuildAggregate(
    cases: list[CaseRecord],
    anomalies: list[AnomalyRecord],
) -> JsonObject:
    by_engine: JsonObject = {}
    for engine in ENGINE_ORDER:
        selected = [case for case in cases if case.engine == engine]
        by_engine[engine] = {
            "case_count": len(selected),
            "case_outcomes": CountsToJson(CountOutcomes(selected), CASE_OUTCOME_ORDER),
            "suspect_slots": CountsToJson(
                CountSuspects(a for a in anomalies if a.engine == engine),
                CLASSIFICATION_ORDER,
            ),
        }

    by_battery_engine: JsonObject = {}
    for battery in BATTERY_ORDER:
        battery_data: JsonObject = {}
        for engine in ENGINE_ORDER:
            selected = [
                case
                for case in cases
                if case.battery == battery and case.engine == engine
            ]
            if selected:
                battery_data[engine] = {
                    "case_count": len(selected),
                    "case_outcomes": CountsToJson(
                        CountOutcomes(selected), CASE_OUTCOME_ORDER
                    ),
                    "suspect_slots": CountsToJson(
                        CountSuspects(
                            a
                            for a in anomalies
                            if a.battery == battery and a.engine == engine
                        ),
                        CLASSIFICATION_ORDER,
                    ),
                }
        by_battery_engine[battery] = battery_data

    line_counts = Counter(
        (a.engine, a.battery, a.test_name, a.classification) for a in anomalies
    )
    case_sets: defaultdict[tuple[str, str, str, str], set[str]] = defaultdict(set)
    for anomaly in anomalies:
        key = (
            anomaly.engine,
            anomaly.battery,
            anomaly.test_name,
            anomaly.classification,
        )
        case_sets[key].add(anomaly.case_id)

    recurrent_suspects: list[object] = []
    for key in sorted(
        case_sets,
        key=lambda item: (len(case_sets[item]), line_counts[item], item),
        reverse=True,
    ):
        engine, battery, test_name, classification = key
        recurrent_suspects.append(
            {
                "engine": engine,
                "battery": battery,
                "test_name": test_name,
                "classification": classification,
                "case_count": len(case_sets[key]),
                "line_count": line_counts[key],
                "case_ids": sorted(case_sets[key]),
            }
        )

    return {
        "schema_version": 1,
        "campaign": {
            "case_count": len(cases),
            "series_counts": dict(Counter(case.series for case in cases)),
            "battery_counts": dict(Counter(case.battery for case in cases)),
            "device_selectors": sorted({case.device_selector for case in cases}),
        },
        "case_outcomes": CountsToJson(CountOutcomes(cases), CASE_OUTCOME_ORDER),
        "suspect_slots": CountsToJson(CountSuspects(anomalies), CLASSIFICATION_ORDER),
        "by_engine": by_engine,
        "by_battery_engine": by_battery_engine,
        "recurrent_suspect_tests": recurrent_suspects,
        "problematic_cases": [
            CaseToJson(case)
            for case in cases
            if case.outcome in {"incomplete", "technical_error"}
        ],
    }


# ------------------------------------------------------------------------------


def CaseToRow(case: CaseRecord) -> CsvRow:
    return cast(CsvRow, CaseToJson(case))


def AnomalyToRow(anomaly: AnomalyRecord) -> CsvRow:
    return cast(CsvRow, AnomalyToJson(anomaly))


def WriteCsv(path: Path, rows: list[CsvRow]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if not rows:
        _ = path.write_text("", encoding="utf-8")
        return

    with path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0].keys()))
        _ = cast(object, writer.writeheader())
        for row in rows:
            _ = cast(object, writer.writerow(row))


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
        print(f"GGEMS TestU01 campaign aggregation failed:\n{error}")
        raise SystemExit(1) from None
