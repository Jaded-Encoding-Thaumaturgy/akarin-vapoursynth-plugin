import pytest
import vapoursynth as vs

core = vs.core


@pytest.mark.parametrize("input_format, input_value, expr, expected", [
    (vs.GRAY8, 0, "0 exp", 1.0),
    (vs.GRAY8, 0, "x exp", 1.0),
    (vs.GRAY8, 1, "x exp", 2.71828),
    (vs.GRAYS, 0.5, "x exp", 1.64872),
])
def test_exp(input_format: vs.VideoFormat, input_value: int, expr: str, expected: float) -> None:
    clip = core.std.BlankClip(format=input_format, color=input_value)
    result = core.akarin.Expr(clip, expr, vs.GRAYS)
    assert result.get_frame(0)[0][0, 0] == pytest.approx(expected)


@pytest.mark.parametrize("input_format, input_value, expr, expected", [
    (vs.GRAY8, 0, "0 log", float('-inf')),
    (vs.GRAY8, 0, "x log", float('-inf')), # differs from std.Expr's -87.3365478515625
    (vs.GRAY8, 1, "x log", 0), 
    (vs.GRAYS, 7.38905, "x log", 2),
])
def test_log(input_format: vs.VideoFormat, input_value: int, expr: str, expected: float) -> None:
    clip = core.std.BlankClip(format=input_format, color=input_value)
    result = core.akarin.Expr(clip, expr, vs.GRAYS)
    assert result.get_frame(0)[0][0, 0] == pytest.approx(expected)


@pytest.mark.parametrize("input_format", [vs.GRAY8, vs.GRAY16, vs.GRAYS])
@pytest.mark.parametrize("expr, expected", [
    ("x 1.5 pow", 0.0),
])
def test_pow(input_format: vs.VideoFormat, expr: str, expected: float) -> None:
    clip = core.std.BlankClip(format=input_format, color=0)
    result = core.akarin.Expr(clip, expr, vs.GRAYS)
    assert result.get_frame(0)[0][0, 0] == pytest.approx(expected)


@pytest.mark.parametrize("input_format, input_value, expr, expected", [
    (vs.GRAY8, 0, "x sin", 0),
    (vs.GRAY8, 1, "x sin", 0.8414709568023682),
    (vs.GRAY8, 2, "x sin", 0.9092974066734314),
])
def test_sin(input_format: vs.VideoFormat, input_value: int, expr: str, expected: float) -> None:
    clip = core.std.BlankClip(format=input_format, color=input_value)
    result = core.akarin.Expr(clip, expr, vs.GRAYS)
    assert result.get_frame(0)[0][0, 0] == pytest.approx(expected)


def test_gh_11() -> None:
    clip = core.std.BlankClip(format=vs.GRAY8, color=0)
    result = core.akarin.Expr(clip, "x 128 / 0.86 pow 255 *")
    assert result.get_frame(0)[0][0, 0] == pytest.approx(6.122468756907559e-31)

    clip = core.std.BlankClip(format=vs.GRAY16, color=0)
    result = core.akarin.Expr(clip, "x 32768 / 0.86 pow 65535 *")
    assert result.get_frame(0)[0][0, 0] == pytest.approx(1.5734745330615421e-28)