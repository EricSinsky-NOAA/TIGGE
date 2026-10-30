#! /bin/env python3
import datetime
import os
import subprocess
import typing
import shutil
from functools import partial
from pathlib import Path
import argparse

# Make sure print statements are flushed immediately, otherwise 
#   print statments may be out-of-order with subprocess output
print = partial(print, flush=True)

cycle_frequency = 6


GEFS_VARIABLE_GROUPS = {
    's': ('0p25', 'p25', range(0, 241, 6)),
	't': ('0p25', 'p25', range(0, 241, 6)), 
}

def check_file_status(path: Path) -> str:
    if not path.is_file():
        return "MISSING"
    if not os.access(path, os.R_OK):
        return "NO_READ_PERM"
    if path.stat().st_size == 0:
        return "EMPTY_0B"
    return "OK"

def get_members(time: datetime.datetime) -> typing.List[str]:
    if time < datetime.datetime(2020, 9, 23, 12):
        n_members = 21
    else:
        n_members = 31

    return list(map(lambda m: f'c{m:02d}' if (m == 0) else f'p{m:02d}', range(0, n_members)))

def get_source_path(source_dir: str, time: datetime.datetime, grib_type: str, path_suffix: str) -> Path:
    if time < datetime.datetime(2020, 9, 23, 12):
        return Path(time.strftime(f'{source_dir}/gefs.%Y%m%d/%H/pgrb2{grib_type}'))
    return Path(time.strftime(f'{source_dir}/gefs.%Y%m%d/%H/atmos/pgrb2{grib_type}{path_suffix}'))


def get_file_path(
    base_dir: Path,
    time: datetime.datetime,
    member: str,
    forecast_hour: int,
    grib_type: str,
    grid_res: str
) -> Path:
    if time < datetime.datetime(2020, 9, 23, 12):
        fname = time.strftime(f'ge{member}.t%Hz.pgrb2{grib_type}f{forecast_hour:02d}')
    else:
        fname = time.strftime(f'ge{member}.t%Hz.pgrb2{grib_type}.{grid_res}.f{forecast_hour:03d}')
    return base_dir / fname

def get_dest_path(out_base: str, time: datetime.datetime) -> Path:
    if time < datetime.datetime(2020, 9, 23, 12):
        # Pre-2020: .../gefs.YYYYMMDD/HH/pgrb2st
        return Path(time.strftime(f'{out_base}/gefs.%Y%m%d/%H/pgrb2st'))
    else:
        # Post-2020: .../gefs.YYYYMMDD/HH/atmos/pgrb2stp25
        return Path(time.strftime(f'{out_base}/gefs.%Y%m%d/%H/atmos/pgrb2stp25'))

def concatenate_grib_pair(source_s: Path, source_t: Path, destination: Path) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    
    with destination.open('wb') as out_f:
        with source_s.open('rb') as f_s:
            shutil.copyfileobj(f_s, out_f, length=1024 * 1024)
        with source_t.open('rb') as f_t:
            shutil.copyfileobj(f_t, out_f, length=1024 * 1024)

def verify_and_combine(
    source_dir: str,
    output_dir: str,
    when: datetime.datetime,
    members: typing.List[str]
) -> None:
    res_s, suffix_s, hours_s = GEFS_VARIABLE_GROUPS['s']
    res_t, suffix_t, hours_t = GEFS_VARIABLE_GROUPS['t']

    dir_s = get_source_path(source_dir, when, 's', suffix_s)
    dir_t = get_source_path(source_dir, when, 't', suffix_t)

    dest_dir = get_dest_path(output_dir, when)

    invalid_count = 0 
    total_pairs = len(members) * len(hours_s)

    print(f"\n{'MEMBER':<8} {'FHR':<6} {'STATUS S':<14} {'STATUS T':<14} {'DESTINATION'}")
    print("-" * 80)

    for member in members:
        for fhr in hours_s:
            path_s = get_file_path(dir_s, when, member, fhr, 's', res_s)
            path_t = get_file_path(dir_t, when, member, fhr, 't', res_t)

            out_filename = path_s.name.replace('pgrb2s', 'pgrb2st') 
            dest_file = dest_dir / out_filename

            s_status = check_file_status(path_s)
            t_status = check_file_status(path_t)

            if s_status != "OK" or t_status != "OK":
                invalid_count += 1
                print(f"{member:<8} {fhr:<6} {s_status:<14} {t_status:<14} [SKIPPED]")
                continue

            # Concatenate
            concatenate_grib_pair(path_s, path_t, dest_file)
            print(f"{member:<8} {fhr:<6} {s_status:<14} {t_status:<14} -> {dest_file}")

    print("-" * 75)
    print(f"Summary: Processed {total_pairs - invalid_count}/{total_pairs} pairs. Missing: {invalid_count}\n")

    if invalid_count > 0:
        print(f"ERROR: Aborting workflow. {invalid_count} required file pair(s) were missing.")
        exit(1)

def main():
    parser = argparse.ArgumentParser(description="Combine pgrb2sp25 and pgrb2tp25 GRIB2 files for TIGGE")
    parser.add_argument('time', type=lambda s: datetime.datetime.strptime(s, "%Y%m%d%H"), help="Cycle date in YYYYMMDDHH format")

    time = parser.parse_args().time

    members = get_members(time=time)

    source_dir = os.environ.get('GEFS_COM')
    out_dir = os.environ.get('GEFS_COM')

    print(time.strftime(f'Checking %Y%m%d_%H'))
    verify_and_combine(source_dir, out_dir, time, members)

if __name__ == "__main__":
    main()       
