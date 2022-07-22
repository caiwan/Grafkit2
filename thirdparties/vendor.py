from collections import defaultdict
from typing import Dict, List
from dataclasses import dataclass, field
# from dataclasses_json import DataClassJsonMixin
from enum import Enum

import sys

# TODO: Use YAML
import json
import argparse
import subprocess
import pathlib
import logging


LOGGER = logging.getLogger(__name__)


class Commands(Enum):
    INIT = 'init'
    BUILD = 'build'
    UPDATE = 'update'
    CLEAR = 'clear'
    HELP = 'help'

command_help_str = {
    Commands.BUILD: "Builds given targets ",
    Commands.UPDATE: "Updates given targets submodule",
    Commands.INIT: "Initializes submodules",
    Commands.CLEAR: "Clears out build workspace",
    Commands.HELP: "This message",
}


@dataclass
class TargetConfig():
    exclude: bool = False
    cmake_config : Dict[str, str] = field(default_factory=dict)

    @staticmethod
    def from_dict(d:dict):
        # Avoids using dacite or dataclass-json
        return TargetConfig(
            exclude = d["exclude"],
            cmake_config = d["cmake_config"]
        )


def get_current_file_dir() -> pathlib.Path:
    return pathlib.Path( __file__).parent


def create_argument_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="CLI tool to invoke arcane magic to obtain coochies")

    parser.add_argument(
        dest="command",
        type=str,
        help="Command to execute.",
    )

    parser.add_argument(
        dest="targets",
        nargs="*",
        type=str,
        help="Targets to execute commands on.",
    )

    parser.add_argument(
        "--config",
        "-c",
        dest="config",
        type=pathlib.Path,
        required=False,
        default= get_current_file_dir() / "config.json",
        help="Configuration file.",
    )

    # TODO Single or multiconfig mode
    parser.add_argument(
        "--build-config",
        "-b",
        dest="build_config",
        type=pathlib.Path,
        required=False,
        default="Relese",
        help="Build configuration",
    )

    parser.add_argument(
        "--force",
        "-f",
        dest="use_force",
        action="store_true",
        required=False,
        default=False,
        help="Enforces certain actions",
    )

    parser.add_argument(
        "--target-path",
        "-t",
        dest="target_path",
        required=False,
        default=pathlib.Path(get_current_file_dir()) / ".vendor",
        help="Target build and install path",
    )

    parser.add_argument(
        "--source-path",
        "-s",
        dest="source_path",
        required=False,
        default=pathlib.Path(get_current_file_dir()),
        help="Source root folder",
    )

    return parser


def fetch_config(config:pathlib.Path) -> Dict[str, TargetConfig]:
    result = {}
    with open(config) as f:
        data = json.load(f)
        for key, value in data.items():
            result[key] = TargetConfig.from_dict(value)
    return result


def exec_command(command: List[str], cwd=None) -> int:
    LOGGER.info(f"Command: {' '.join(command)}")
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=cwd)
    returncode = process.poll()
    while returncode is None:
        try:
            stdout_data, stderr_data = process.communicate(timeout=60)  # TODO: Configure T.O.

        except subprocess.TimeoutExpired:
            process.kill()
            stdout_data, stderr_data = process.communicate()
            return -1

        if stdout_data:
            LOGGER.info(stdout_data.decode("UTF-8"))
        if stderr_data:
            LOGGER.error(stderr_data.decode("UTF-8"))

        returncode = process.poll()

    return returncode


def init(source_path:pathlib.Path):
    # TODO use custom source file path from cfg
    exec_command ("git submodule update --init --recursive".split(), cwd=source_path)


def update(source_path:pathlib.Path):
    # TODO use custom source file path from cfg
    # Set cwd to submodule repo root
    # git submodule update --remote --merge
    pass


def build(target: str, target_config:Dict[str, str], target_path:pathlib.Path, source_path:pathlib.Path):
    LOGGER.info(f"Building target={target}")

    source_path = source_path / target
    build_path = target_path / "build" / target
    install_path = target_path / "install"

    all(dir.mkdir(parents=True, exist_ok=True) for dir in [build_path, install_path])

    cmake_config = " ".join([f"-D{key}={value}" for key, value in target_config.cmake_config.items()])

    exec_command(f"cmake -S {source_path} -B {build_path} -DCMAKE_INSTALL_PREFIX={install_path} {cmake_config}".split())
    # TODO: Multi or single config
    exec_command(f"cmake --build {build_path} --target Install --config Release".split())


def clear(target: str, use_force:bool, target_path:pathlib.Path):
    LOGGER.info(f"Clearing target={target}")
    try:
        if not use_force:
            (target_path / "build" / target / "CMakeCache.txt").unlink()
        else:
            (target_path / "build" / target).unlink()
    except FileNotFoundError:
        pass


def help(config):
    create_argument_parser().print_help()
    glue = "\n  "
    f = lambda e : f"{e.value} - {command_help_str[e]}"
    LOGGER.info(f"\nAvailable commands: {glue}{glue.join([f(e) for e in Commands])}")
    LOGGER.info(f"\nAvailable targets: {glue}{glue.join([k for k in config.keys()])}")


def fetch_targets(args, config:Dict[str, TargetConfig]):
    if 'all' in args.targets:
        for target, target_config in config.items():
            if not target_config.exclude:
                yield target, target_config
    else:
        for target in args.targets:
            if target in config:
                yield target, config[target]


def main():
    args = create_argument_parser().parse_args()
    config = fetch_config(args.config)

    if args.command not in [e.value for e in Commands]:
        help(config)

    else:
        command = Commands(args.command)

        if command == Commands.INIT:
            init(args.source_path)
        elif command == Commands.HELP:
            help(config)
        else:
            for target, target_config in fetch_targets(args, config):
                if command == Commands.BUILD:
                    build(target, target_config, args.target_path, args.source_path)
                elif command == Commands.CLEAR:
                    clear(target, args.use_force, args.target_path)
                elif command == Commands.UPDATE:
                    update(target, args.source_path)


if __name__ == "__main__":
    root = logging.getLogger()
    root.setLevel(logging.DEBUG)

    handler = logging.StreamHandler(sys.stdout)
    handler.setLevel(logging.DEBUG)
    formatter = logging.Formatter('%(message)s')
    handler.setFormatter(formatter)
    root.addHandler(handler)

    main()
